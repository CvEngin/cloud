#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout> // 垂直布局
#include <QHBoxLayout> // 水平布局
#include "protocol.h"
#include <QTimer>
#include "sharefile.h"

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void flushFileList(PDU *pdu);
    void clearEnterDir();
    QString getEnterDir();
    void setDownloadStatus(bool status);

    qint64 m_iTotal = 0;    // 下载的文件总大小
    qint64 m_iReceived = 0; // 已经下载的文件大小

    bool getDownloadStatus();
    QString getFileSavePath();
    QString getShareFileName();

signals:

public slots:
    void createDir();
    void flushFile();
    void deleteDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPreContent();
    void uploadFile();
    void uploadFileData();
    void deleteFile();

    void downloadFile();
    void shareFile();

    void moveFile();
    void selectDestDir();

private:
    QListWidget *m_pFileListWidget;         // 文件列表
    QPushButton *m_pReturnPB;               // 返回主页面
    QPushButton *m_pCreateDirPB;            // 新建文件夹
    QPushButton *m_pDeleteDirPB;            // 删除文件夹
    QPushButton *m_pRenameFilePB;           // 重命名文件
    QPushButton *m_pFlushFilePB;            // 刷新文件
    QPushButton *m_pUploadFilePB;           // 上传文件
    QPushButton *m_pDownLoadFilePB;         // 下载文件
    QPushButton *m_pDeleteFilePB;           // 删除文件
    QPushButton *m_pShareFilePB;            // 分享文件
    QPushButton *m_pMoveFilePB;             // 移动文件
    QPushButton *m_pSelectMoveToDirPB;      // 移动文件到其他文件夹

    QString m_strEnterDir;          // 存储要进去的文件夹信息
    QString m_strUploadFilePath;    // 存储上传的文件信息
    QString m_strFileSavePath;      // 存储下载文件的保存路径
    QString m_strShareFileName;     // 存储共享的文件信息
    QString m_strMoveFileName;      // 存储移动的文件信息
    QString m_strMoveFilePath;      // 存储移动文件的源路径
    QString m_strDestDirPath;       // 存储移动文件的目标路径

    QTimer *m_pTimer;  // 定时器
    bool m_downloadStatus;  // 文件下载状态
};

#endif // BOOK_H
