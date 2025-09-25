#include "new_chat.h"
#include "ui_new_chat.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QCheckBox>

New_chat::New_chat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::New_chat)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    //socket->connectToHost("localhost", 1234);
    socket->connectToHost("26.13.49.181", 1234);
}

New_chat::~New_chat()
{
    delete ui;
}
void New_chat::setUserList(const QStringList &users)
{
    userList = users;
    ui->userList->clear();

    for (const QString &user : users) {
        if (user != currentUserId) {
        QListWidgetItem *item = new QListWidgetItem(user, ui->userList);
        item->setCheckState(Qt::Unchecked);
        }
    }
}

void New_chat::on_createButton_clicked()
{
    QString title = ui->titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Chat title cannot be empty");
        return;
    }

    QStringList selectedUsers;
    for (int i = 0; i < ui->userList->count(); ++i) {
        QListWidgetItem *item = ui->userList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedUsers.append(item->text());
        }
    }

    if (selectedUsers.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Select at least one user");
        return;
    }
    QString currentUser = getCurrentUserId();
    QJsonObject request;
    request["action"] = "createChat";
    request["title"] = title;
    request["admin"] = currentUserId;
    QJsonArray usersArray;
    for (const QString &user : selectedUsers) {
        usersArray.append(user);
    }
    request["users"] = usersArray;
    sendRequest(request);
    emit chatCreated(title, currentUser, selectedUsers);
    this->close();
}
void New_chat::setCurrentUserId(const QString &userId) {
    currentUserId = userId;
}

QString New_chat::getCurrentUserId() const {
    return currentUserId;
}
void New_chat::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}
