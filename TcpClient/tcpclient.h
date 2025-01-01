#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QMainWindow>
#include <QString>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"


// 登录界面
QT_BEGIN_NAMESPACE
namespace Ui {
class TcpClient;
}
QT_END_NAMESPACE

class TcpClient : public QMainWindow
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();

    static TcpClient &getInstance();
    QTcpSocket &getTcpSokcet();

public slots:
    void showConnect();
    void recvMsg();

private slots:
    // void on_send_pb_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;

    // 连接服务器，和服务器数据交互
    QTcpSocket m_tcpSocket;
};
#endif // TCPCLIENT_H
