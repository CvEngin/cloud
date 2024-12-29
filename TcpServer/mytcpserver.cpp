#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer() {

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(handle);
    m_tcpSocketList.append(pTcpSocket);
}
