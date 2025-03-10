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
    // m_db.setDatabaseName("G:\\cloud\\cloud.db");
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

// 查找用户状态信息
int OpeDB::handleSearchUsr(const char *name)
{
    if (name == NULL) return -1;
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        return query.value(0).toInt();
    } else {
        return -1;
    }
}

// 添加好友前检索好友信息是否存在
int OpeDB::handleAddFriend(const char *perName, const char *loginName)
{
    if (perName == NULL || loginName == NULL) return -1;
    QString data = QString("select * from friendInfo where "
                           "(id = (select id from usrInfo where name = \'%1\') "
                           "and friendId = (select id from usrInfo where name = \'%2\')) "
                           "or (id = (select id from usrInfo where name = \'%3\') "
                           "and friendId = (select id from usrInfo where name = \'%4\'))")
                       .arg(perName).arg(loginName).arg(loginName).arg(perName);
    QSqlQuery query;
    query.exec(data);
    if (query.next()) {
        return 0;  // 双方已是好友
    } else {
        data = QString("select online from usrInfo where name = \'%1\'").arg(perName);
        QSqlQuery query_2;
        query_2.exec(data);
        if (query_2.next()) {
            // 1 表示这个人在线，2 表示离线，3表示不存在这个人
            return query_2.value(0).toInt() == 1 ? 1 : 2;
        } else {
            return 3;
        }
    }
}

// 通过name找id
int OpeDB::getIdByUserName(const char *name)
{
    if(NULL == name)
    {
        return -1;
    }
    QString data = QString("select id from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    // qDebug() << data;
    if(query.next())
    {
        return query.value(0).toInt();
    }
    else
    {
        return -1; // 不存在该用户
    }
}

// 增加好友
void OpeDB::handleAddFriendAgree(const char *perName, const char *loginName)
{
    if(NULL == perName || NULL == loginName) return;
    int sourceUserId = getIdByUserName(loginName);
    int addedUserId = getIdByUserName(perName);
    // qDebug() << sourceUserId << " " << addedUserId;
    QString strQuery = QString("insert into friendInfo values(%1, %2) ").arg(sourceUserId).arg(addedUserId);
    QSqlQuery query;
    query.exec(strQuery);
    return;
}

// 搜索好友列表
QStringList OpeDB::handleFlushFriend(const char *sourceName)
{
    QStringList strFriendList;
    strFriendList.clear();
    if (sourceName == NULL) return strFriendList;
    QString data = QString("select * from usrInfo "
                           "where id in (select id from friendInfo where friendId=(select id from usrInfo where name=\'%1\')) "
                           "or id in (select friendId from friendInfo where id=(select id from usrInfo where name=\'%2\'))")
                       .arg(sourceName).arg(sourceName);
    // qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    while (query.next()) {
        // QString userMes = query.value(1).toString() + (query.value(3).toBool() ? " (在线)" : " (离线)");
        // qDebug() << userMes;
        // strFriendList.append(userMes);
        strFriendList.append(query.value(1).toString());
        strFriendList.append(query.value(3).toString());
    }
    return strFriendList;
}

// 删除好友
bool OpeDB::handleDelFriend(const char *perName, const char *loginName)
{
    if (perName == NULL || loginName == NULL) return false;
    int sourceId = getIdByUserName(loginName);
    int deleteId = getIdByUserName(perName); // 请求方name对应的id
    // qDebug() << sourceId << deleteId << perName;
    QString data = QString("delete from friendInfo where (id=\'%1\' and friendId=\'%2\') or (id=\'%3\' and friendId =\'%4\')")
                       .arg(sourceId).arg(deleteId).arg(deleteId).arg(sourceId);
    QSqlQuery query;
    query.exec(data);
    return true;
}

// 群发消息时查找在线好友
QStringList OpeDB::handleGroupChat(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(NULL == name)
    {
        return strFriendList;
    }
    strFriendList = handleFlushFriend(name);
    strFriendList.append(name);
    strFriendList.append("在线");
    return strFriendList;
}
