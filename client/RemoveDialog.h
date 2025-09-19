#ifndef REMOVEDIALOG_H
#define REMOVEDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QTcpSocket>

namespace Ui {
class RemoveDialog;
}

class RemoveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveDialog(QWidget *parent = nullptr);
    ~RemoveDialog();
    void setUserList(const QStringList &users);
    void setCurrentChatTitle(const QString &chatTitle);
    QString getCurrentChatTitle() const;
signals:
    void userRemoved(const QString &title, const QStringList &selectedUsers);
private slots:
    void on_remove_clicked();
private:
    Ui::RemoveDialog *ui;
    QTcpSocket *socket;
    QStringList userList;
    QString currentChatTitle;
    void sendRequest(const QJsonObject &request);
};

#endif // REMOVEDIALOG_H
