#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("D:\\sourceCode\\cloud\\cloud.db");
    if (m_db.open()) {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next()) {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString())
                               .arg(query.value(1).toString())
                               .arg(query.value(2).toString());
            // qDebug() << data;
        }
    } else {
        QMessageBox::critical(NULL, "打开数据库", "打开数据库失败");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}


// 处理注册
bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL) return false;
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\',\'%2\')")
                       .arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

// 处理登录
bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL) return false;
    QString data = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0")
                       .arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        data = QString("update usrInfo set online = 1 where name = \'%1\' and pwd = \'%2\'")
                    .arg(name).arg(pwd);
        query.exec(data);
        return true;
    } else {
        return false;
    }
}

// 处理用户退出登录功能
void OpeDB::handleOffline(const char *name)
{
    if (name == NULL) return;
    QString data = QString("update usrInfo set online = 0 where name = \'%1\'")
                       .arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online = 1");
    QSqlQuery query;
    query.exec(data);
    QStringList res;
    res.clear();
    while (query.next()) {
        res.append(query.value(0).toString());
    }
    return res;
}
