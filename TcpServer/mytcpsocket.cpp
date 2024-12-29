#include "mytcpsocket.h"
#include <QDebug>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead())
        , this, SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    qDebug() << "总数据大小：" << this->bytesAvailable(); // 打印接收到的大小
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint)); // 获取总的大小
    uint uiMsgLen = uiPDULen - sizeof(PDU);  // 计算实际的消息长度
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
    qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
}
