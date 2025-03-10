#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"

Online::Online(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

// 将在线用户显示在列表中
void Online::showUsr(PDU *pdu)
{
    if (pdu == NULL) return;
    uint uiSize = pdu->uiMsgLen / 32;
    char caTmp[32];
    ui->online_lw->clear();
    for(uint i = 0; i < uiSize; i++){
        memcpy(caTmp, (char*)(pdu->caMsg) + i * 32, 32);
        // 补充：不显示自己信息，防止之后添加自己为好友等操作错误
        if(strcmp(caTmp, TcpClient::getInstance().strLoginName().toStdString().c_str()) == 0)
        {
            continue;
        }
        ui->online_lw->addItem(caTmp);
    }
}

// 添加好友按钮
void Online::on_addFriend_pb_clicked()
{
    QString strPerUsrName = ui->online_lw->currentItem()->text();
    QString strLoginName = TcpClient::getInstance().strLoginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strPerUsrName.toStdString().c_str(), strPerUsrName.size());
    memcpy(pdu->caData + 32, strLoginName.toStdString().c_str(), strLoginName.size());
    TcpClient::getInstance().getTcpSokcet().write((char*) pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

