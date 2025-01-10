#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include "privatechat.h"

TcpClient::TcpClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected())
            , this, SLOT(showConnect()));
    connect(&m_tcpSocket, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (file.open(QIODeviceBase::ReadOnly)) {
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();
        strData.replace("\n", " ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << m_strIP << " " << m_usPort;
    } else {
        QMessageBox::critical(this, "Open Config", "Open Config failed");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSokcet()
{
    return m_tcpSocket;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::recvMsg()
{
    uint uiPDULen = 0;
    m_tcpSocket.read((char*)&uiPDULen, sizeof(uint)); // 获取总的大小
    uint uiMsgLen = uiPDULen - sizeof(PDU);  // 计算实际的消息长度
    PDU *pdu = mkPDU(uiMsgLen);
    m_tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
    switch(pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_RESPOND: {
        if (strcmp(pdu->caData, REGIST_OK) == 0) {
            QMessageBox::information(this, "注册", REGIST_OK);
        } else if (strcmp(pdu->caData, REGIST_FAILED) == 0) {
            QMessageBox::information(this, "注册", REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND: {
        if (strcmp(pdu->caData, LOGIN_OK) == 0) {
            QMessageBox::information(this, "登录", LOGIN_OK);
            OpeWidget::getInstance().show();
            this->hide();
        } else if (strcmp(pdu->caData, LOGIN_FAILED) == 0) {
            QMessageBox::information(this, "登录", LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND: {
        OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND: {
        QMessageBox::information(this, "搜索", QString("%1: %2").arg(OpeWidget::getInstance().getFriend()->m_strSearchName).arg(pdu->caData));
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST: {
        char caName[32] = {'\0'};
        strncpy(caName, pdu->caData + 32, 32);
        int res = QMessageBox::information(this, "好友申请", QString("%1 want to add you").arg(caName)
                                 , QMessageBox::Ok, QMessageBox::No);

        PDU * respdu = mkPDU(0);
        memcpy(respdu->caData, pdu->caData, 32);
        memcpy(respdu->caData + 32, pdu->caData + 32, 32);
        if (res == QMessageBox::Ok) {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        } else {
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND: {
        QMessageBox::information(this, "添加好友", pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE: {
        QMessageBox::information(this, "添加好友", QString("%1 已同意您的好友申请！").arg(pdu->caData));
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
        QMessageBox::information(this, "添加好友", QString("%1 已拒绝您的好友申请！").arg(pdu->caData));
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND: {
        OpeWidget::getInstance().getFriend()->flushFriendList(pdu);
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
    {
        if(0 == strcmp(pdu->caData, DEL_FRIEND_OK))
        {
            QMessageBox::information(this, "删除好友", pdu->caData);
        }
        else if(0 == strcmp(pdu -> caData, DEL_FRIEND_ERROR))
        {
            QMessageBox::warning(this, "删除好友", pdu->caData);
        }
        break;
    }
    case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: // 处理服务器转发过来的删除好友请求
    {
        char sourceName[32] = {'\0'}; // 获取发送方用户名
        strncpy(sourceName, pdu->caData + 32, 32);
        QMessageBox::information(this, "删除好友", QString("%1 已解除与您的好友关系！").arg(sourceName));
        break;
    }
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST : // 处理服务器转发过来的私聊请求
    {
        if(PrivateChat::getInstance().isHidden())
        {
            PrivateChat::getInstance().show();
        }
        char caSendName[32] = {'\0'}; // 获取发送方用户名
        strncpy(caSendName, pdu->caData, 32);
        QString sendName = caSendName;
        PrivateChat::getInstance().setChatName(sendName);
        PrivateChat::getInstance().updateMsg(pdu);
        break;
    }
    default: break;
    }
    free(pdu);
    pdu = NULL;
}

#if 0
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (!strMsg.isEmpty()) {
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    } else {
        QMessageBox::warning(this, "信息发送", "发送的数据不能为空");
    }
}
#endif


// 登录点击事件
void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    } else {
        QMessageBox::critical(this, "登录", "登录失败:用户名或密码为空");
    }
}


// 注册点击事件
void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData + 32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    } else {
        QMessageBox::critical(this, "注册", "注册失败:用户名或密码为空");
    }
}


void TcpClient::on_cancel_pb_clicked()
{

}

// 获取登录的用户名
QString TcpClient::strLoginName() const
{
    return m_strLoginName;
}

