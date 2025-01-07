#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

// 数据库操作封装
class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

    bool handleRegist(const char *name, const char *pwd);
    bool handleLogin(const char *name, const char *pwd);
    void handleOffline(const char *name);
    QStringList handleAllOnline();
    int handleSearchUsr(const char *name);
    int handleAddFriend(const char *perName, const char *loginName);
    int getIdByUserName(const char *name);
    void handleAddFriendAgree(const char *perName, const char *loginName);
    QStringList handleFlushFriend(const char *sourceName);

signals:

private:
    QSqlDatabase m_db;  // 连接数据库
};

#endif // OPEDB_H
