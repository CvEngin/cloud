#include "sharefile.h"
#include "tcpclient.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");
    m_pConfirmPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");
    m_pSA = new QScrollArea;                         // 显示好友的区域
    m_pFriendW = new QWidget;                        // 显示好友的 Widget
    m_pButtonGroup = new  QButtonGroup(m_pFriendW);  // 用于管理所有好友，放在 Widget 里面
    m_pButtonGroup->setExclusive(false);
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pConfirmPB);
    pDownHBL->addWidget(m_pCancelPB);
    pDownHBL->addStretch();

    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    connect(m_pCancelSelectPB, SIGNAL(clicked(bool)), this, SLOT(cancelSelect()));
    connect(m_pSelectAllPB, SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(m_pConfirmPB, SIGNAL(clicked(bool)), this, SLOT(shareConfirm()));
    connect(m_pCancelPB, SIGNAL(clicked(bool)), this, SLOT(shareCancel()));
}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

// 更新好友列表
void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if (pFriendList == NULL) return;
    // 清除掉以前的
    QAbstractButton *tmp;
    QList<QAbstractButton*> preFriendList = m_pButtonGroup->buttons();
    for(int i = 0; i < preFriendList.size(); i++) {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    // 刷新出新的
    QCheckBox *pCB;
    for (int i = 0; i < pFriendList->count(); i++) {
        // qDebug() << "好友名字" << pFriendList->item(i)->text();
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); i++) {
        if(cbList[i]->isChecked()) {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for(int i = 0; i < cbList.size(); i++) {
        if(!cbList[i]->isChecked()) {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::shareConfirm()
{
    QString strName = TcpClient::getInstance().strLoginName();
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    QString shareFileName = OpeWidget::getInstance().getBook()->getShareFileName();
    QString strSharePath = strCurPath + "/" + shareFileName; //完整的文件路径
    // qDebug() << strSharePath;
    QList<QAbstractButton*> cbList =  m_pButtonGroup->buttons();
    int shareNum = 0;
    for(int i = 0; i < cbList.size(); i++)
        if(cbList[i]->isChecked()) shareNum++;
    PDU *pdu = mkPDU(32 * shareNum + strSharePath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData, "%s %d", strName.toStdString().c_str(), shareNum);
    int j = 0;
    for (int i = 0; i < cbList.size(); i++) {
        if (cbList[i]->isChecked()) {
            memcpy((char*)(pdu->caMsg) + 32 * j, cbList[i]->text().split('\t')[0].toStdString().c_str(), cbList[i]->text().split('\t')[0].size());
            j++;
        }
    }
    memcpy((char*)(pdu->caMsg) + shareNum * 32, strSharePath.toStdString().c_str(), strSharePath.size());
    TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
    hide();
}

void ShareFile::shareCancel()
{
    hide();
}
