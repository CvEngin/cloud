#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"

// 信号处理
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();

signals:
    void offline(MyTcpSocket *mySocket);

public slots:
    void recvMsg();
    void clientOffline();

private:
    QString m_strName;
};

#endif // MYTCPSOCKET_H
