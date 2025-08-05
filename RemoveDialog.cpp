#include "RemoveDialog.h"
#include "ui_RemoveDialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QCheckBox>

RemoveDialog::RemoveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveDialog)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    socket->connectToHost("localhost", 1234);
}

RemoveDialog::~RemoveDialog()
{
    delete ui;
}
void RemoveDialog::setUserList(const QStringList &users)
{
    userList = users;
    ui->usersList->clear();
    for (const QString &user : users) {
        QListWidgetItem *item = new QListWidgetItem(user, ui->usersList);
        item->setCheckState(Qt::Unchecked);
    }
}

void RemoveDialog::on_remove_clicked()
{
    QStringList selectedUsers;
    for (int i = 0; i < ui->usersList->count(); ++i) {
        QListWidgetItem *item = ui->usersList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedUsers.append(item->text());
        }
    }

    if (selectedUsers.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Select at least one user");
        return;
    }
    QString currentChatTitle = getCurrentChatTitle();
    QJsonObject request;
    request["action"] = "removeUserFromChat";
    request["chatTitle"] = currentChatTitle;
    QJsonArray usersArray;
    for (const QString &user : selectedUsers) {
        usersArray.append(user);
    }
    request["users"] = usersArray;
    sendRequest(request);
    emit userRemoved(currentChatTitle, selectedUsers);
    this->close();
}
QString RemoveDialog::getCurrentChatTitle() const {
    return currentChatTitle;
}
void RemoveDialog::setCurrentChatTitle(const QString &chatTitle) {
    currentChatTitle = chatTitle;
}
void RemoveDialog::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}
