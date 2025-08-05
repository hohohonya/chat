#include "adddialog.h"
#include "ui_adddialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QCheckBox>


AddDialog::AddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDialog)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    socket->connectToHost("localhost", 1234);
}

AddDialog::~AddDialog()
{
    delete ui;
}
void AddDialog::setUserList(const QStringList &users)
{
    userList = users;
    ui->usersList->clear();
    for (const QString &user : users) {
        QListWidgetItem *item = new QListWidgetItem(user, ui->usersList);
        item->setCheckState(Qt::Unchecked);
    }
}

void AddDialog::on_add_clicked()
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
    request["action"] = "addUsersToChat";
    request["chatTitle"] = currentChatTitle;
    QJsonArray usersArray;
    for (const QString &user : selectedUsers) {
        usersArray.append(user);
    }
    request["users"] = usersArray;
    sendRequest(request);
    emit userAdded(currentChatTitle, selectedUsers);
    this->close();
}
QString AddDialog::getCurrentChatTitle() const {
    return currentChatTitle;
}
void AddDialog::setCurrentChatTitle(const QString &chatTitle) {
    currentChatTitle = chatTitle;
}
void AddDialog::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}
