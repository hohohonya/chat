#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QTcpSocket>

namespace Ui {
class AddDialog;
}

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddDialog(QWidget *parent = nullptr);
    ~AddDialog();
    void setUserList(const QStringList &users);
    void setCurrentChatTitle(const QString &chatTitle);
    QString getCurrentChatTitle() const;
signals:
    void userAdded(const QString &title, const QStringList &selectedUsers);
private slots:
    void on_add_clicked();
private:
    Ui::AddDialog *ui;
    QTcpSocket *socket;
    QStringList userList;
    QString currentChatTitle;
    void sendRequest(const QJsonObject &request);

};

#endif // ADDDIALOG_H
