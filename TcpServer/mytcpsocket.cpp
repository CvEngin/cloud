#include "mytcpsocket.h"
#include <QDebug>
#include <mytcpserver.h>
#include <QDir>
#include <QFileInfoList>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, SIGNAL(readyRead())
            , this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected())
            , this, SLOT(clientOffline()));

    m_bUpload = false;
    m_pTimer = new QTimer;

    connect(m_pTimer, SIGNAL(timeout())
            , this, SLOT(sendFileToClient()));
}


// 获取socket的名字，用于测试
QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);
    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();
    QString srcTmp;
    QString destTmp;
    for(int i = 0; i < fileInfoList.size(); i++) {
        if(fileInfoList[i].isFile()) {
            srcTmp += strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp, destTmp);
        }
        else if(fileInfoList[i].isDir()) {
            if(QString(".") == fileInfoList[i].fileName() || QString("..") == fileInfoList[i].fileName())
                continue;
            srcTmp += strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            copyDir(srcTmp, destTmp);
        }
    }
}

// 接受消息并处理
void MyTcpSocket::recvMsg()
{
    if (!m_bUpload) {
        // qDebug() << "总数据大小：" << this->bytesAvailable(); // 打印接收到的大小
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint)); // 获取总的大小
        uint uiMsgLen = uiPDULen - sizeof(PDU);  // 计算实际的消息长度
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
        switch(pdu->uiMsgType) {
        // 注册请求
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
                QDir dir;
                qDebug() << "create dir: " << dir.mkdir(QString("./%1").arg(caName));
            } else {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 登录请求
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
        // 获取在线用户请求
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
        // 搜索用户请求
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
        // 增加好友请求
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
        // 同意增加好友
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
        // 拒绝增加好友
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE: {
            char sourceName[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(sourceName, pdu -> caData + 32, 32);
            pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            // 服务器需要转发给发送好友请求方其被拒绝的消息
            MyTcpServer::getInstance().resend(sourceName, pdu);
            break;
        }
        // 刷新好友列表
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
        // 删除好友
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST: {
            char delName[32] = {'\0'};
            char sourceName[32] = {'\0'};
            // 拷贝读取的信息
            strncpy(delName, pdu->caData, 32);
            strncpy(sourceName, pdu->caData + 32, 32);
            bool ret = OpeDB::getInstance().handleDelFriend(delName, sourceName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            if(ret) strcpy(respdu->caData, DEL_FRIEND_OK);
            else strcpy(respdu->caData, DEL_FRIEND_ERROR);
            MyTcpServer::getInstance().resend(delName, pdu);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 私聊
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char sourceName[32] = {'\0'};
            char chatName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            strncpy(chatName, pdu->caData + 32, 32);
            MyTcpServer::getInstance().resend(chatName, pdu);
            MyTcpServer::getInstance().resend(sourceName, pdu);
            break;
        }
        // 群聊
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST: {
            char sourceName[32] = {'\0'};
            strncpy(sourceName, pdu->caData, 32);
            QStringList onlineFriend = OpeDB::getInstance().handleGroupChat(sourceName);
            QString tmp;
            for(int i = 0; i * 2 < onlineFriend.size(); i++)
            {
                tmp = onlineFriend.at(i * 2);
                MyTcpServer::getInstance().resend(tmp.toStdString().c_str(), pdu);
            }
            break;
        }
        // 创建文件夹
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST: {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)pdu->caMsg);
            bool ret = dir.exists(QString(strCurPath));
            // qDebug() << "strcurpath: " << strCurPath;
            PDU *respdu = mkPDU(0);
            if(ret)  // 当前目录存在
            {
                char strNewPath[32] = {'\0'};
                memcpy(strNewPath, pdu->caData + 32, 32);
                QString newPath = strCurPath + "/" + strNewPath;
                // qDebug() << newPath;
                bool isExist = dir.exists(newPath);
                if(isExist)  // 要创建的新目录已经存在
                {
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, DIR_ALREADY_EXSIT);
                }
                else
                {
                    dir.mkdir(newPath);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(respdu->caData, CREATE_DIR_OK);
                }
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(respdu->caData, DIR_N0_EXSIT);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 刷新文件列表
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST: {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);    //当前的路径
            // qDebug() << "刷新请求的路径：" << pCurPath;
            QDir dir(pCurPath);
            QFileInfoList fileList = dir.entryInfoList();
            int fileCount = fileList.size();
            PDU *respdu = mkPDU(sizeof(FileInfo) * (fileCount));
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            //遍历当前路径下所有的文件和文件夹
            for(int i = 0; i < fileList.size(); i++){
                //qDebug() << fileList.at(i).fileName() << " " << fileList.at(i).size()
                //        << "文件夹:" << fileList.at(i).isDir() << "常规文件：" << fileList.at(i).isFile();
                pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                strFileName = fileList[i].fileName();
                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                if(fileList[i].isDir())
                {
                    pFileInfo->iFileType = 0;
                }
                else if(fileList[i].isFile())
                {
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 删除文件夹
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST: {
            char strDirName[32] = {'\0'};
            strcpy(strDirName, pdu->caData);  // 取出要删除的文件夹名字
            char *pDirPath = new char[pdu->uiMsgLen];
            memcpy(pDirPath, pdu->caMsg, pdu->uiMsgLen);  // 取出路径
            QString strPath = QString("%1/%2").arg(pDirPath).arg(strDirName);  // 拼接成完整路径
            // qDebug() << strPath;
            bool ret = false;
            QFileInfo fileInfo(strPath);
            if(fileInfo.isDir()) {
                QDir dir;
                dir.setPath(strPath);
                ret = dir.removeRecursively();
            } else if(fileInfo.isFile()){
                ret = false;
            }
            PDU *respdu;
            if(ret){
                respdu = mkPDU(strlen(DIR_DELETE_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                memcpy(respdu->caData, DIR_DELETE_OK, strlen(DIR_DELETE_OK));
            } else {
                respdu = mkPDU(strlen(DIR_DELETE_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
                memcpy(respdu->caData, DIR_DELETE_FAILED, strlen(DIR_DELETE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 重命名文件
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST: {
            char oldFileName[32] = {'\0'};
            char newFileName[32] = {'\0'};
            strncpy(oldFileName, pdu->caData, 32);
            strncpy(newFileName, pdu->caData + 32, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(oldFileName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(newFileName);
            bool ret = false;
            QDir dir;
            ret = dir.rename(strOldPath, strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if (ret) memcpy(respdu->caData, RENAME_FILE_OK, strlen(RENAME_FILE_OK));
            else memcpy(respdu->caData, RENAME_FILE_FAILED, strlen(RENAME_FILE_FAILED));
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 进入文件夹
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST: {
            char caEnterDirName[32] = {'\0'};
            strncpy(caEnterDirName, pdu->caData, 32);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caEnterDirName);
            // qDebug() << strNewPath;
            QFileInfo fileInfo(strNewPath);
            if(fileInfo.isDir()) {
                QDir dir(strNewPath);
                QFileInfoList fileList = dir.entryInfoList();
                int fileCount = fileList.size();
                PDU *respdu = mkPDU(sizeof(FileInfo) * (fileCount));
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo;
                QString strFileName;
                for(int i = 0; i < fileList.size(); i++){
                    pFileInfo = (FileInfo*)(respdu->caMsg) + i;
                    strFileName = fileList[i].fileName();
                    memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.size());
                    if(fileList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileList[i].isFile())
                    {
                        pFileInfo->iFileType = 1;
                    }
                }
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            } else if (fileInfo.isFile()) {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                memcpy(respdu->caData, ENTER_DIR_FAILED, strlen(ENTER_DIR_FAILED));
                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = NULL;
            }
            break;
        }
        // 上传文件
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST: {
            char uploadFileName[32] = {'\0'};
            qint64 uploadFileSize = 0;
            sscanf(pdu->caData, "%s %lld", uploadFileName, &uploadFileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(uploadFileName);
            // qDebug() << strNewPath;
            delete []pPath;
            pPath = NULL;

            m_file.setFileName(strNewPath);
            if(m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = uploadFileSize;
                m_iReceived = 0;
            }
            break;
        }
        // 删除文件
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST: {
            char strFileName[32] = {'\0'};
            strcpy(strFileName, pdu->caData);
            char *pDirPath = new char[pdu->uiMsgLen];
            memcpy(pDirPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pDirPath).arg(strFileName);
            // qDebug() << strPath;
            bool ret = false;
            QFileInfo fileInfo(strPath);
            if(fileInfo.isDir()) {
                ret = false;
            }
            else if(fileInfo.isFile()) {
                QDir dir;
                ret = dir.remove(strPath);
            }
            PDU *respdu;
            if(ret) {
                respdu = mkPDU(strlen(FILE_DELETE_OK) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                memcpy(respdu->caData, FILE_DELETE_OK, strlen(FILE_DELETE_OK));
            }
            else {
                respdu = mkPDU(strlen(FILE_DELETE_FAILED) + 1);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
                memcpy(respdu->caData, FILE_DELETE_FAILED, strlen(FILE_DELETE_FAILED));
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        // 下载文件
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST: {
            char caFileName[32] = {'\0'};
            strcpy(caFileName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caFileName);

            // qDebug() << "当前下载文件的地址："  << strNewPath;
            delete []pPath;
            pPath = NULL;

            QFileInfo fileInfo(strNewPath);
            qint64 fileSize = fileInfo.size();
            // qDebug() << "caFileName" << caFileName << "filesize" << fileSize;
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caData, "%s %lld", caFileName, fileSize);

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            m_file.setFileName(strNewPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            break;
        }
        // 共享文件
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST: {
            char strSendName[32] = {'\0'};
            int shareNum = 0;
            sscanf(pdu->caData, "%s %d", strSendName, &shareNum);
            qDebug() << "分享文件的人：" << strSendName << " 人数：" << shareNum;
            int size = shareNum * 32;
            PDU *respdu = mkPDU(pdu->uiMsgLen - size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_REQUEST;
            strcpy(respdu->caData, strSendName);
            memcpy(respdu->caMsg, (char*)(pdu->caMsg) + size, pdu->uiMsgLen - size); //文件路径
            qDebug() << "接收到文件的路径为：" << respdu->caMsg;
            char caReceiveName[32] = {'\0'};
            for (int i = 0; i < shareNum; i++) {
                memcpy(caReceiveName, (char*)pdu->caMsg + i * 32, 32);
                qDebug() << "接收到文件的好友为：" << caReceiveName;
                MyTcpServer::getInstance().resend(caReceiveName, respdu);
            }
            free(respdu);
            respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData, SHARE_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND: {
            QString strReceivePath = QString("./%1").arg(pdu->caData);  // 被分享者的路径
            QString strSharePath = QString("%1").arg((char*)(pdu->caMsg));  // 被分享文件的路径
            int index = strSharePath.lastIndexOf('/');
            QString strFileName = strSharePath.right(strSharePath.size() - index - 1);  // 被分享的文件名
            strReceivePath = strReceivePath + '/' + strFileName;
            QFileInfo fileInfo(strSharePath);
            if(fileInfo.isDir()) {
                copyDir(strSharePath, strReceivePath);
            }
            else if(fileInfo.isFile()) {
                QFile::copy(strSharePath, strReceivePath);
            }
            break;
        }
        // 移动文件
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST: {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData, "%d%d%s", &srcLen, &destLen, caFileName);
            char *pSrcPath = new char[srcLen + 1];
            char *pDestPath = new char[destLen + 1 + 32];  // 这里需要指定移动过来的那个文件的名字，所以需要 +32
            memset(pSrcPath, '\0', srcLen + 1);
            memset(pDestPath, '\0', destLen + 1 + 32);

            memcpy(pSrcPath, pdu->caMsg, srcLen);
            memcpy(pDestPath, (char*)(pdu->caMsg) + (srcLen + 1), destLen);

            // qDebug() << pSrcPath << " " << pDestPath;

            QFileInfo fileInfo(pDestPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir()) {
                strcat(pDestPath, "/");
                strcat(pDestPath, caFileName);

                bool res = QFile::rename(pSrcPath, pDestPath);
                if(res) {
                    strcpy(respdu->caData, MOVE_FILE_OK);
                }
                else {
                    strcpy(respdu->caData, COMMON_ERROR);
                }

            }
            else if(fileInfo.isFile()) {
                strcpy(respdu->caData, MOVE_FILE_FAILED);
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
    } else {
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        QByteArray buffer = readAll();
        m_file.write(buffer);
        m_iReceived += buffer.size();
        if(m_iTotal == m_iReceived) {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_OK);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(m_iTotal < m_iReceived) {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData, UPLOAD_FILE_FAILED);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
    }
}

// 处理客户端下线信号
void MyTcpSocket::clientOffline()
{
    // 将数据库中的 online 改为 0
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    // 释放下线信号给 mytcpserver
    emit offline(this);
}

// 将客户端要下载的文件传过去
void MyTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char *buffer = new char[4096];
    qint64 ret = 0;
    while(true) {
        ret = m_file.read(buffer, 4096);
        if(ret > 0 && ret <= 4096) {
            write(buffer, ret);
        }
        else if(0 == ret) {
            m_file.close();
            break;
        }
        else if(ret < 0) {
            qDebug() << "发送文件内容给Client失败";
            m_file.close();
            break;
        }
    }
    delete []buffer;
    buffer = NULL;
}
