#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer() {

}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}

// 接受客户端的消息
void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "new client connected";
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(handle);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket*))
            , this, SLOT(deleteSocket(MyTcpSocket*)));
}

// 通过socket向指定用户名的客户端发送信息
void MyTcpServer::resend(const char *perName, PDU *pdu)
{
    if (perName == NULL || pdu == NULL) return;
    QString strName = perName;
    for (int i = 0; i < m_tcpSocketList.size(); i++) {
        if (strName == m_tcpSocketList.at(i)->getName()) {
            m_tcpSocketList.at(i)->write((char*) pdu, pdu->uiPDULen);
            break;
        }
    }
}

// 用户下线后，根据用户名删除其socket
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
