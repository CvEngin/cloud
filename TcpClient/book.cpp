#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_strEnterDir.clear();
    m_downloadStatus = false;
    m_pTimer = new QTimer;

    m_pFileListWidget = new QListWidget;                  // 文件列表

    m_pReturnPB = new QPushButton("返回");                  // 返回主页面
    m_pCreateDirPB = new QPushButton("新建文件夹");            // 新建文件夹
    m_pDeleteDirPB = new QPushButton("删除文件夹");           // 删除文件夹
    m_pRenameFilePB = new QPushButton("重命名文件");          // 重命名文件
    m_pFlushFilePB = new QPushButton("刷新文件");            // 刷新文件

    m_pUploadFilePB = new QPushButton("上传文件");           // 上传文件
    m_pDownLoadFilePB = new QPushButton("下载文件");        // 下载文件
    m_pDeleteFilePB = new QPushButton("删除文件");          // 删除文件
    m_pShareFilePB = new QPushButton("分享文件");           // 分享文件
    m_pMoveFilePB = new QPushButton("移动文件");             //移动文件
    m_pSelectMoveToDirPB = new QPushButton("目标目录");      //移动文件到其他文件夹
    m_pSelectMoveToDirPB->setEnabled(false);

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDeleteDirPB);
    pDirVBL->addWidget(m_pRenameFilePB);
    pDirVBL->addWidget(m_pFlushFilePB);


    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadFilePB);
    pFileVBL->addWidget(m_pDownLoadFilePB);
    pFileVBL->addWidget(m_pDeleteFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectMoveToDirPB);


    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pFileListWidget);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB, SIGNAL(clicked(bool)), this, SLOT(createDir()));
    connect(m_pFlushFilePB, SIGNAL(clicked(bool)), this, SLOT(flushFile()));
    connect(m_pDeleteDirPB, SIGNAL(clicked(bool)), this, SLOT(deleteDir()));
    connect(m_pRenameFilePB, SIGNAL(clicked(bool)), this, SLOT(renameFile()));
    connect(m_pFileListWidget ,SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnPreContent()));
    connect(m_pUploadFilePB, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(uploadFileData()));
    connect(m_pDeleteFilePB, SIGNAL(clicked(bool)), this, SLOT(deleteFile()));
    connect(m_pDownLoadFilePB, SIGNAL(clicked(bool)), this, SLOT(downloadFile()));
    connect(m_pShareFilePB, SIGNAL(clicked(bool)), this, SLOT(shareFile()));
    connect(m_pMoveFilePB, SIGNAL(clicked(bool)), this, SLOT(moveFile()));
    connect(m_pSelectMoveToDirPB, SIGNAL(clicked(bool)), this, SLOT(selectDestDir()));
}

// 刷新文件列表函数（将服务器上的文件信息展示）
void Book::flushFileList(PDU *pdu)
{
    if (pdu == NULL) return;
    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen / sizeof(FileInfo);
    m_pFileListWidget->clear();
    for(int i = 2; i < iCount ; i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg) + i;
        // qDebug() << pFileInfo->caFileName << " " << pFileInfo->iFileType;
        QListWidgetItem *pItem = new QListWidgetItem;
        if(pFileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.png")));
        }
        else if(pFileInfo->iFileType == 1)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/file.png")));
        }
        pItem->setText(pFileInfo->caFileName);
        m_pFileListWidget->addItem(pItem);
    }
    return;
}

// 文件夹进入失败或拼接完成后，清空当前存储的文件夹信息
void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

// 获取进入的文件夹信息用于更新当前文件路径
QString Book::getEnterDir()
{
    return m_strEnterDir;
}

// 设置下载状态
void Book::setDownloadStatus(bool status)
{
    m_downloadStatus = status;
}

// 获取下载状态
bool Book::getDownloadStatus()
{
    return m_downloadStatus;
}

// 获取保存的文件路径
QString Book::getFileSavePath()
{
    return m_strFileSavePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

// 新建文件夹按钮
void Book::createDir()
{
    QString newDirName = QInputDialog::getText(this, "新建文件夹", "文件夹名字: ");
    if (newDirName.isEmpty())
    {
        QMessageBox::warning(this, "新建文件夹", "文件夹名不能为空");
        return;
    }
    else if(newDirName.size() >= 32)
    {
        QMessageBox::warning(this, "新建文件夹", "too long");
    }
    QString strLoginName = TcpClient::getInstance().strLoginName();
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
    strncpy(pdu->caData, strLoginName.toStdString().c_str(), 32);
    strncpy(pdu->caData + 32, newDirName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 刷新文件按钮
void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 删除文件夹按钮
void Book::deleteDir()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件夹", "请选择删除的文件夹");
    }
    else
    {
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        QString deleteName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
        strncpy((char*)(pdu->caData), deleteName.toStdString().c_str(), deleteName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

// 重命名文件夹按钮
void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "重命名文件", "请选择重命名的文件");
    }
    else
    {
        QString newFileName = QInputDialog::getText(this, "重命名文件", "文件新名字：");
        QString oldFileName = pItem->text();
        if(newFileName.isEmpty())
        {
            QMessageBox::warning(this, "重命名文件", "文件名不能为空");
        }
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
        strncpy((char*)(pdu->caData), oldFileName.toStdString().c_str(), oldFileName.size());
        strncpy((char*)(pdu->caData + 32), newFileName.toStdString().c_str(), newFileName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

// 进入文件夹
void Book::enterDir(const QModelIndex &index)
{
    QString selectDirName = index.data().toString();
    m_strEnterDir = selectDirName;
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    PDU *pdu = mkPDU(strCurPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    // qDebug() << selectDirName;
    strncpy(pdu->caData, selectDirName.toStdString().c_str(), selectDirName.size());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

// 返回上一级文件夹按钮
void Book::returnPreContent()
{
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    QString strRootPath = "./" + TcpClient::getInstance().strLoginName();
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "返回上一级目录", "已经在根目录");
    }
    else
    {
        //"目录的格式： ./aa/bb/cc"
        int index = strCurPath.lastIndexOf('/');
        QString newPath = strCurPath.remove(index, strCurPath.size() - index);
        // qDebug() << "上一级目录为：" << newPath;
        TcpClient::getInstance().setCurPath(newPath);
        flushFile();
    }
}

// 上传文件按钮
void Book::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().getM_strCurPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    // qDebug() << m_strUploadFilePath;
    if (m_strUploadFilePath == nullptr) {
        QMessageBox::warning(this, "上传文件", "文件不能为空");
    }
    else{
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString newFileName = m_strUploadFilePath.right(m_strUploadFilePath.size() - index - 1);
        // qDebug() << newFileName;
        QFile file(m_strUploadFilePath);
        qint64 uploadFileSize = file.size();
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", newFileName.toStdString().c_str(), uploadFileSize);
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

        m_pTimer->start(1000);
    }
}

// 上传文件到服务器（定时器结束后进行）
void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "上传文件", "打开文件失败");
        return;
    }
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true) {
        ret = file.read(pBuffer, 4096);
        if(ret > 0 && ret <= 4096) {
            TcpClient::getInstance().getTcpSokcet().write(pBuffer, ret);
        }
        else if(ret == 0) {
            break;
        }
        else {
            QMessageBox::warning(this, "上传文件", "上传文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

// 删除常规文件按钮
void Book::deleteFile()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem)
    {
        QMessageBox::warning(this, "删除文件", "请选择删除的文件");
    }
    else
    {
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        QString deleteName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
        strncpy((char*)(pdu->caData), deleteName.toStdString().c_str(), deleteName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

// 下载文件按钮
void Book::downloadFile()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem) {
        QMessageBox::warning(this, "下载文件", "请选择下载的文件");
    }
    else {
        QString strFileSavePath = QFileDialog::getSaveFileName();
        if(strFileSavePath.isEmpty()) {
            QMessageBox::warning(this, "下载文件", "请选择文件保存位置");
            m_strFileSavePath.clear();
        }
        else {
            m_strFileSavePath = strFileSavePath;
            // qDebug() << "文件保存的位置：" << m_strFileSavePath;
        }
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        QString downloadName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        strcpy(pdu->caData, downloadName.toStdString().c_str());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem) {
        QMessageBox::warning(this, "分享文件", "请选择分享的文件");
        return;
    }
    else {
        m_strShareFileName = pItem->text();
        // qDebug() << "选中的文件为： " << m_strShareFileName;
    }
    QListWidget *pFriendList = OpeWidget::getInstance().getFriend()->getFriendList();
    ShareFile::getInstance().updateFriend(pFriendList);
    // for (int i = 0; i < pFriendList->count(); i++) qDebug() << pFriendList->item(i);
    if(ShareFile::getInstance().isHidden()) {
        ShareFile::getInstance().show();
    }

}

void Book::moveFile()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem) {
        QMessageBox::warning(this, "移动文件", "请选择移动的文件");
    }
    else {
        m_strMoveFileName = pItem->text();
        // qDebug() << "移动的文件为： " << m_strMoveFileName;
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        m_strMoveFilePath = strCurPath + '/' + m_strMoveFileName;
        m_pSelectMoveToDirPB->setEnabled(true);
    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pItem = m_pFileListWidget->currentItem();
    if(NULL == pItem) {
        QMessageBox::warning(this, "移动文件", "请选择移动到的文件夹");
    }
    else
    {
        QString destDirName = pItem->text();
        QString strCurPath = TcpClient::getInstance().getM_strCurPath();
        m_strDestDirPath = strCurPath + '/' + destDirName;
        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strDestDirPath.size();
        PDU *pdu = mkPDU(srcLen + destLen + 2);
        pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData, "%d %d %s", srcLen, destLen, m_strMoveFileName.toStdString().c_str());
        memcpy(pdu->caMsg, m_strMoveFilePath.toStdString().c_str(), srcLen);
        memcpy((char*)(pdu->caMsg) + (srcLen + 1), m_strDestDirPath.toStdString().c_str(), destLen);
        TcpClient::getInstance().getTcpSokcet().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    m_pSelectMoveToDirPB->setEnabled(false);
}
