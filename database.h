#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    bool connectToDatabase();
    bool authenticateUser(const QString &username, const QString &password);
    bool registerUser(const QString &name, const QString &surname, const QString &username, const QString &password);
    QVector<QPair<QString, QString>> getUserChats(const QString &username);
    QVector<QVector<QString>> getChatMessages(const QString &chatTitle);
    bool createChat(const QString &title, const QString &admin, const QStringList &users);
    bool addUserToChat(const QString &chatTitle, const QString &username,const QString &admin);
    bool addUsersToChat(const QString &chatTitle, const QStringList &username);
    bool removeUserFromChat(const QString &chatTitle, const QString &username);
    bool removeUsersFromChat(const QString &chatTitle,const QStringList &username);
    bool sendMessage(const QString &chatTitle, const QString &username, const QString &message);
    bool isUserAdmin(const QString &chatTitle, const QString &username);
    QVector<QString> getAllUsers();
    QString whoIsAdmin(const QString &chatTitle);
    QStringList getChatUsers(const QString &chatTitle);
    QStringList getNotChatUsers(const QString &chatTitle);

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
