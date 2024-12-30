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

    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*))
            , this, SLOT(deleteSocket(MyTcpSocket*)));
}

void MyTcpServer::deleteSocket(MyTcpSocket *mySocket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for (; iter != m_tcpSocketList.end(); iter++) {
        // 释放对应socket的空间并将其从列表中删除
        if (mySocket == *iter) {
            delete *iter;
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    // 打印出名字看socket是否在下线后被删除
    // for (int i = 0; i < m_tcpSocketList.size(); i++) {
    //     qDebug() << m_tcpSocketList.at(i)->getName();
    // }
}
