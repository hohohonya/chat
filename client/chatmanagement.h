#ifndef CHATMANAGEMENT_H
#define CHATMANAGEMENT_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class chatmanagement;
}

class chatmanagement : public QDialog
{
    Q_OBJECT

public:
    explicit chatmanagement(QWidget *parent = nullptr);
    ~chatmanagement();
    void setUserList(const QStringList &users);

signals:
    void userAdded();
    void userRemoved();

private slots:
    void on_add_clicked();
    void on_remove_clicked();

private:
    Ui::chatmanagement *ui;
    QString chatTitle;
    QString currentUser;
    QString admin;
    QStringList userList;
    void sendRequest(const QJsonObject &request);
};

#endif // CHATMANAGEMENT_H
