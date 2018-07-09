#pragma once

#include <cinttypes>
#include <string>
#include <mutex>
#include <functional>
#include <array>
#include <thread>
#include <map>

namespace Ac {

constexpr int8_t DELIMITER = 0xaa;

enum CMD :int8_t{
    DWRITE=0x01,
    AWRITE,
    PMODE,
    DELAY,
    ANSWER
};
enum DW :int8_t{
    HIGH=0x01,
    LOW
};
enum PM :int8_t{
    OUTPUT=0x01,
    INPUT
};
enum MISC :int8_t{
    TRUE=0x01,
    FALSE
};

class Arduino
{
public:
    using SENDERTYPE = std::function<void(const char*,int)>;
    using RECEIVERTYPE = std::function<void(char*,int)>;

    enum class ERROR{
        NOERROR=0,
        BADREPLY,
        NOREPLY
    };

    explicit Arduino(const std::string &comport=""){
        if(!comport.empty()){
            //Use default send and receive
        }
    }
    void setSendHandler(SENDERTYPE send){
        m_send = send;
    }
    void setReceiveHandler(RECEIVERTYPE recv){
        m_receive = recv;
    }

    void digitalWrite(int pin,DW st)
    {
        const char buf[] = {static_cast<char>(DELIMITER),
                          static_cast<char>(CMD::DWRITE),
                          static_cast<char>(pin),
                          static_cast<char>(st)
                          };
        handleSend(buf,4);
    }

    void pinMode(int pin,PM st)
    {
        const char buf[] = {static_cast<char>(DELIMITER),
                              static_cast<char>(CMD::PMODE),
                              static_cast<char>(pin),
                              static_cast<char>(st)
                              };
        handleSend(buf,4);
    }

    void analogWrite(int pin,int val){
        const char buf[] = {static_cast<char>(DELIMITER),
                              static_cast<char>(CMD::AWRITE),
                              static_cast<char>(pin),
                              static_cast<char>(val)
                              };
        handleSend(buf,4);
    }

    //void delay(int millis);

    std::string errorStr()const{
        return m_errorsDesc.at(m_err);
    }
    ERROR error()const{
        return m_err;
    }
private:

    bool handleSend(const char *buf, int size){
        std::lock_guard<std::mutex> guard(m_sendMutex);
        setError();
        m_send(buf,size);

        //Must wait for return from arduino board and then return
        ERROR recvError = ERROR::NOREPLY;
        std::array<char,1>inBuf;
        int counter = 50;
        while(counter--){
            m_receive(inBuf.data(),inBuf.size());
            if(inBuf[0]==static_cast<char>(DELIMITER)){
                return true;
            }else if(inBuf[0]!=char{}){
                recvError = ERROR::BADREPLY;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        setError(recvError);
        return false;
    }

    void init(){
        m_errorsDesc[ERROR::NOERROR] = "";
        m_errorsDesc[ERROR::NOREPLY] = "NOREPLY";
    }

    void setError(ERROR err=ERROR::NOERROR){
        m_err = err;
    }

    ERROR m_err=ERROR::NOERROR;

    std::mutex m_sendMutex;
    SENDERTYPE m_send;
    RECEIVERTYPE m_receive;
    std::map<ERROR,std::string> m_errorsDesc;

};

}
