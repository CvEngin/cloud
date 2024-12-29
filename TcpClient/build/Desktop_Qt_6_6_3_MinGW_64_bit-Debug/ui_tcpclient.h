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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TcpClient
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *pwd_lab;
    QLabel *name_lab;
    QLineEdit *pwd_le;
    QLineEdit *name_le;
    QPushButton *login_pb;
    QHBoxLayout *horizontalLayout;
    QPushButton *regist_pb;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancel_pb;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TcpClient)
    {
        if (TcpClient->objectName().isEmpty())
            TcpClient->setObjectName("TcpClient");
        TcpClient->resize(275, 191);
        centralwidget = new QWidget(TcpClient);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");
        pwd_lab = new QLabel(centralwidget);
        pwd_lab->setObjectName("pwd_lab");
        QFont font;
        font.setPointSize(14);
        pwd_lab->setFont(font);

        gridLayout->addWidget(pwd_lab, 1, 0, 1, 1);

        name_lab = new QLabel(centralwidget);
        name_lab->setObjectName("name_lab");
        name_lab->setFont(font);

        gridLayout->addWidget(name_lab, 0, 0, 1, 1);

        pwd_le = new QLineEdit(centralwidget);
        pwd_le->setObjectName("pwd_le");
        pwd_le->setFont(font);
        pwd_le->setEchoMode(QLineEdit::EchoMode::Password);

        gridLayout->addWidget(pwd_le, 1, 1, 1, 1);

        name_le = new QLineEdit(centralwidget);
        name_le->setObjectName("name_le");
        name_le->setFont(font);

        gridLayout->addWidget(name_le, 0, 1, 1, 1);

        login_pb = new QPushButton(centralwidget);
        login_pb->setObjectName("login_pb");
        QFont font1;
        font1.setPointSize(16);
        login_pb->setFont(font1);

        gridLayout->addWidget(login_pb, 2, 0, 1, 2);


        verticalLayout->addLayout(gridLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        regist_pb = new QPushButton(centralwidget);
        regist_pb->setObjectName("regist_pb");

        horizontalLayout->addWidget(regist_pb);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancel_pb = new QPushButton(centralwidget);
        cancel_pb->setObjectName("cancel_pb");

        horizontalLayout->addWidget(cancel_pb);


        verticalLayout->addLayout(horizontalLayout);


        horizontalLayout_2->addLayout(verticalLayout);

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
        pwd_lab->setText(QCoreApplication::translate("TcpClient", "\345\257\206\347\240\201\357\274\232", nullptr));
        name_lab->setText(QCoreApplication::translate("TcpClient", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        login_pb->setText(QCoreApplication::translate("TcpClient", "\347\231\273\345\275\225", nullptr));
        regist_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\345\206\214", nullptr));
        cancel_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\351\224\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TcpClient: public Ui_TcpClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
