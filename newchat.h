#ifndef NEWCHAT_H
#define NEWCHAT_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class NewChat;
}

class NewChat : public QDialog
{
    Q_OBJECT

public:
    explicit NewChat(QWidget *parent = nullptr);
    ~NewChat();

    void setUserList(const QStringList &users);

signals:
    void chatCreated(const QString &title);

private slots:
    void on_createButton_clicked();

private:
    Ui::NewChat *ui;
    QStringList userList;
};

#endif // NEWCHAT_H
