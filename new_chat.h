#ifndef NEW_CHAT_H
#define NEW_CHAT_H

#include <QDialog>
#include <QStringList>
#include <QTcpSocket>

namespace Ui {
class New_chat;
}

class New_chat : public QDialog
{
    Q_OBJECT

public:
    explicit New_chat(QWidget *parent = nullptr);
    ~New_chat();
    void setCurrentUserId(const QString &userId);
    QString getCurrentUserId() const;
    void setUserList(const QStringList &users);
signals:
    void chatCreated(const QString &title,const QString &currentUser, const QStringList &selectedUsers);
private slots:
    void on_createButton_clicked();

private:
    Ui::New_chat *ui;
    QTcpSocket *socket;
    QStringList userList;
    QString currentUserId;
    void sendRequest(const QJsonObject &request);
};

#endif // NEW_CHAT_H
