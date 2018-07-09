#include <iostream>
#include <QSerialPort>
#include "arduino-controller.h"
using namespace std;

void initQtSerialPort(QSerialPort &sp,const QString &portName){
    sp.setPortName(portName);
    sp.setBaudRate(QSerialPort::Baud115200);
    if(!sp.open(QSerialPort::ReadWrite)){
        qFatal("Unable to open com port!!");
    }
}

int main()
{
    cout << "Start!" << endl;
    QSerialPort sp;
    initQtSerialPort(sp,"com20");
    Ac::Arduino arduino;

    arduino.setSendHandler([&sp](const char*data,int size)->int{
        return sp.write(data,size);
    });
    arduino.setReceiveHandler([&sp](char*data,int size)->int{
        return sp.read(data,size);
    });

    return 0;
}
