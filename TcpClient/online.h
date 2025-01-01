#ifndef ONLINE_H
#define ONLINE_H

#include <QWidget>
#include "protocol.h"

// 显示在线好友界面
namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();

    void showUsr(PDU *pdu);

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
