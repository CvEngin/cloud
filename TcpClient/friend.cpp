#include "friend.h"
#include "tcpclient.h"
#include <QInputDialog>

Friend::Friend(QWidget *parent)
    : QWidget{parent}
{
    m_pshowMsgTE = new QTextEdit; // 信息展示框
    m_pfrindListWidget = new QListWidget;  // 好友列表
    m_pInputMsgLE = new QLineEdit;

    m_pDelFriendPB = new QPushButton("删除好友");
    m_pflushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUsrPB = new QPushButton("显示在线用户");
    m_pSearchUsrPB = new QPushButton("查找用户");
    m_pMsgSendPB = new QPushButton("信息发送");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDelFriendPB);
    pRightPBVBL->addWidget(m_pflushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUsrPB);
    pRightPBVBL->addWidget(m_pSearchUsrPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);


    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pshowMsgTE);
    pTopHBL->setStretch(0, 2);  // 设置 m_pshowMsgTE 的伸缩因子为 2
    pTopHBL->addWidget(m_pfrindListWidget);
    pTopHBL->setStretch(1, 1);  // 设置 m_pfrindListWidget 的伸缩因子为 1
    pTopHBL->addLayout(pRightPBVBL);

    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    m_pOnline = new Online;

    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pTopHBL);
    pMain->addLayout(pMsgHBL);
    pMain->addWidget(m_pOnline);
    m_pOnline->hide();

    setLayout(pMain);

    connect(m_pShowOnlineUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(showOnline()));
    connect(m_pSearchUsrPB, SIGNAL(clicked(bool))
            , this, SLOT(searchUsr()));
    connect(m_pflushFriendPB, SIGNAL(clicked(bool))
            , this, SLOT(flushFriend()));
}

// 显示在线用户到列表中
void Friend::showAllOnlineUsr(PDU *pdu)
{
    if (pdu == NULL) return;
    m_pOnline->showUsr(pdu);
}

// 刷新好友列表
void Friend::flushFriendList(PDU *pdu)
{
    m_pfrindListWidget->clear();
    if (pdu == NULL) return;

    char caName[32] = {'\0'};
    char onlineStatus[4] = {'\0'};
    uint strSize = pdu->uiMsgLen / 36;

    m_pfrindListWidget->clear();
    for(uint i = 0; i < strSize ; i++)
    {
        memcpy(caName, (char*)(pdu->caMsg) + i * 36, 32);
        memcpy(onlineStatus, (char*)(pdu->caMsg) + 32 + i * 36, 4);
        // qDebug() << "客户端好友" << caName << " " << onlineStatus;
        m_pfrindListWidget->addItem(QString("%1\t%2").arg(caName)
                                         .arg(strcmp(onlineStatus, "1") == 0?"在线":"离线"));
    }
    return;
}

// 显示在线用户按钮
void Friend::showOnline()
{
    if (m_pOnline->isHidden()) {
        m_pOnline->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    } else {
        m_pOnline->hide();
    }
}

// 搜索好友按钮
void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名: ");
    if (!m_strSearchName.isEmpty()) {
        PDU *pdu = mkPDU(0);
        memcpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.size());
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}


// 刷新好友列表按钮
void Friend::flushFriend()
{
    QString strName = TcpClient::getInstance().strLoginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), strName.size());
    TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}




