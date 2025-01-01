#include "mytcpsocket.h"
#include <QDebug>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOffline()));
}


// 获取socket的名字，用于测试
QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::recvMsg()
{
    // qDebug() << "总数据大小：" << this->bytesAvailable(); // 打印接收到的大小
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint)); // 获取总的大小
    uint uiMsgLen = uiPDULen - sizeof(PDU);  // 计算实际的消息长度
    PDU *pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
    switch(pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST: {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData + 32, 32);
        bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if (ret) {
            strcpy(respdu->caData, REGIST_OK);
        } else {
            strcpy(respdu->caData, REGIST_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST: {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName, pdu->caData, 32);
        strncpy(caPwd, pdu->caData + 32, 32);
        bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if (ret) {
            strcpy(respdu->caData, LOGIN_OK);
            m_strName = caName;
        } else {
            strcpy(respdu->caData, LOGIN_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST: {
        QStringList res = OpeDB::getInstance().handleAllOnline();
        uint uiMsgLen = res.size() * 32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for (int i = 0; i < res.size(); i++) {
            memcpy((char*)(respdu->caMsg) + i * 32, res.at(i).toStdString().c_str(), res.at(i).size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    default: break;
    }
    free(pdu);
    pdu = NULL;
}


// 处理客户端下线信号
void MyTcpSocket::clientOffline()
{
    // 将数据库中的 online 改为 0
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    // 释放下线信号给 mytcpserver
    emit offline(this);
}
