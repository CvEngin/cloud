/********************************************************************************
** Form generated from reading UI file 'tcpclient.ui'
**
** Created by: Qt User Interface Compiler version 6.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPCLIENT_H
#define UI_TCPCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TcpClient
{
public:
    QWidget *centralwidget;
    QLineEdit *lineEdit;
    QPushButton *send_pb;
    QLineEdit *lineEdit_2;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TcpClient)
    {
        if (TcpClient->objectName().isEmpty())
            TcpClient->setObjectName("TcpClient");
        TcpClient->resize(800, 600);
        centralwidget = new QWidget(TcpClient);
        centralwidget->setObjectName("centralwidget");
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(30, 420, 651, 41));
        send_pb = new QPushButton(centralwidget);
        send_pb->setObjectName("send_pb");
        send_pb->setGeometry(QRect(690, 420, 81, 41));
        QFont font;
        font.setPointSize(16);
        send_pb->setFont(font);
        lineEdit_2 = new QLineEdit(centralwidget);
        lineEdit_2->setObjectName("lineEdit_2");
        lineEdit_2->setGeometry(QRect(30, 10, 741, 401));
        QFont font1;
        font1.setPointSize(14);
        lineEdit_2->setFont(font1);
        TcpClient->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(TcpClient);
        statusbar->setObjectName("statusbar");
        TcpClient->setStatusBar(statusbar);

        retranslateUi(TcpClient);

        QMetaObject::connectSlotsByName(TcpClient);
    } // setupUi

    void retranslateUi(QMainWindow *TcpClient)
    {
        TcpClient->setWindowTitle(QCoreApplication::translate("TcpClient", "TcpClient", nullptr));
        send_pb->setText(QCoreApplication::translate("TcpClient", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TcpClient: public Ui_TcpClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
