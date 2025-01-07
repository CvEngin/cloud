#include "mytcpsocket.h"
#include <QDebug>
#include <mytcpserver.h>

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
        bool res = OpeDB::getInstance().handleLogin(caName, caPwd);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if (res) {
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
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST: {
        int res = OpeDB::getInstance().handleSearchUsr(pdu->caData);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if (res == -1) {
            strcpy(respdu->caData, SEARCH_USR_NO);
        } else if (res == 1) {
            strcpy(respdu->caData, SEARCH_USR_ONLINE);
        } else {
            strcpy(respdu->caData, SEARCH_USR_OFFLINE);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        char caPerName[32] = {'\0'};
        char caLoginName[32] = {'\0'};
        strncpy(caPerName, pdu->caData, 32);
        strncpy(caLoginName, pdu->caData + 32, 32);
        int ret = OpeDB::getInstance().handleAddFriend(caPerName, caLoginName);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        if (ret == -1) {
            strcpy(respdu->caData, UNKNOW_ERROR);
        } else if (ret == 0) {
            strcpy(respdu->caData, EXIST_FRIEND);
        } else if (ret == 1) {
            MyTcpServer::getInstance().resend(caPerName, pdu);
        } else if (ret == 2) {
            strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
        } else {
            strcpy(respdu->caData, ADD_FRIEND_NOEXIST);
        }
        if (ret != 1) {
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: {
        char addedName[32] = {'\0'};
        char sourceName[32] = {'\0'};
        // 拷贝读取的信息
        strncpy(addedName, pdu->caData, 32);
        strncpy(sourceName, pdu->caData + 32, 32);

        pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        // 将新的好友关系信息写入数据库
        OpeDB::getInstance().handleAddFriendAgree(addedName, sourceName);
        // 服务器需要转发给发送好友请求方其被同意的消息
        MyTcpServer::getInstance().resend(sourceName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
        char sourceName[32] = {'\0'};
        // 拷贝读取的信息
        strncpy(sourceName, pdu -> caData + 32, 32);
        pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        // 服务器需要转发给发送好友请求方其被拒绝的消息
        MyTcpServer::getInstance().resend(sourceName, pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST: {
        char sourceName[32] = {'\0'};
        // 拷贝读取的信息
        strncpy(sourceName, pdu -> caData, 32);
        QStringList res = OpeDB::getInstance().handleFlushFriend(sourceName);
        uint uiMsgLen = res.size() / 2 * 36;  // 36 char[32] 好友名字 + 4 int 在线状态
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for (int i = 0; i * 2 < res.size(); i++) {
            memcpy((char*)(respdu->caMsg) + 36 * i, res.at(i * 2).toStdString().c_str(), 32);
            memcpy((char*)(respdu->caMsg) + 36 * i + 32, res.at(i * 2 + 1).toStdString().c_str(), 4);
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
