#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include <QDir>
#include <QFile>
#include <QTimer>

// 信号处理
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();
    void copyDir(QString strSrcDir, QString strDestDir);

signals:
    void offline(MyTcpSocket *mySocket);

public slots:
    void recvMsg();
    void clientOffline();
    void sendFileToClient();

private:
    QString m_strName;
    QFile m_file;  // 上传或下载的文件
    qint64 m_iTotal;  // 上传文件的大小
    qint64 m_iReceived;  // 判断当前上传了多少
    bool m_bUpload;  // 判断当前状态是否是上传文件状态

    QTimer *m_pTimer;  // 定时器
};

#endif // MYTCPSOCKET_H
