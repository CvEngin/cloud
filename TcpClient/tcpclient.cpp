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
    if (!OpeWidget::getInstance().getBook()->getDownloadStatus()) {
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
                // QMessageBox::information(this, "登录", LOGIN_OK);
                m_strCurPath = QString("./%1").arg(m_strLoginName);
                qDebug() << LOGIN_OK;
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
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST: // 处理服务器转发过来的私聊请求
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
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            OpeWidget::getInstance().getFriend()->updateGroupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND: {
            QMessageBox::information(this, "创建文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND: {
            OpeWidget::getInstance().getBook()->flushFileList(pdu);
            // 进入新文件夹，更新当前所在路径
            QString strEnterDir = OpeWidget::getInstance().getBook()->getEnterDir();
            if (!strEnterDir.isEmpty()) {
                m_strCurPath = m_strCurPath + "/" + strEnterDir;
                OpeWidget::getInstance().getBook()->clearEnterDir();
            }
            // qDebug() << m_strCurPath;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND: {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND: {
            QMessageBox::information(this, "重命名文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND: {
            OpeWidget::getInstance().getBook()->clearEnterDir();
            QMessageBox::information(this, "进入文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND: {
            QMessageBox::information(this, "上传文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_RESPOND: {
            QMessageBox::information(this, "删除文件夹", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND: {
            qDebug() << "pdu->caData:" <<  pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            // qDebug() << "respond:" << caFileName << " " << OpeWidget::getInstance().getBook()->m_iTotal;
            if(strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0) {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                QString strFileSavePath = OpeWidget::getInstance().getBook()->getFileSavePath();
                m_file.setFileName(strFileSavePath);
                if(!m_file.open(QIODevice::WriteOnly)) {
                    QMessageBox::warning(this, "下载文件", "获取保存文件路径失败");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND: {
            QMessageBox::information(this, "共享文件", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST: {
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            char *pos = strrchr(pPath, '/');
            if (pos != NULL) {
                pos++;
                QString strNote = QString("%1 share file->%2 \n Do you accept?").arg(pdu->caData).arg(pos);
                int res = QMessageBox::question(this, "共享文件", strNote);
                if(res == QMessageBox::Yes) {
                    PDU *respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);
                    QString strName = TcpClient::getInstance().strLoginName();
                    // qDebug() << "被分享者：" << strName;
                    strcpy(respdu->caData, strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
                }
            } else {
                qDebug() << "error";
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND: {
            QMessageBox::information(this, "移动文件", MOVE_FILE_OK);
            break;
        }
        default: break;
        }
        free(pdu);
        pdu = NULL;
    } else {
        Book *pBook = OpeWidget::getInstance().getBook();
        QByteArray buffer = m_tcpSocket.readAll();
        m_file.write(buffer);
        pBook->m_iReceived += buffer.size();
        qDebug() << "buffer:" << buffer.size();
        if(pBook->m_iTotal == pBook->m_iReceived) {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iReceived = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::information(this, "下载文件", "下载文件成功");
        }
        else if(pBook->m_iTotal < pBook->m_iReceived) {
            qDebug() << "总共的：" << pBook->m_iTotal << " 下载的" << pBook->m_iReceived;
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iReceived = 0;
            pBook->setDownloadStatus(false);
            QMessageBox::critical(this, "下载文件", "下载文件失败");
        }
    }
}

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

QString TcpClient::getM_strCurPath() const
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString strCurPath)
{
    m_strCurPath = strCurPath;
}

