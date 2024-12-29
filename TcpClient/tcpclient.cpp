#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();

    connect(&m_tcpSocket, SIGNAL(connected())
            , this, SLOT(showConnect()));
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
        strData.replace("\r\n", " ");
        QStringList strList = strData.split(" ");
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << m_strIP << " " << m_usPort;
    } else {
        QMessageBox::critical(this, "Open Config", "Open Config failed");
    }
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
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


void TcpClient::on_login_pb_clicked()
{

}


void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REQUEST;
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

