#include "newchat.h"
#include "ui_newchat.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QCheckBox>

NewChat::NewChat(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewChat)
{
    ui->setupUi(this);
}

NewChat::~NewChat()
{
    delete ui;
}

void NewChat::setUserList(const QStringList &users)
{
    userList = users;
    ui->usersList->clear();

    for (const QString &user : users) {
        QListWidgetItem *item = new QListWidgetItem(user, ui->usersList);
        item->setCheckState(Qt::Unchecked);
    }
}

void NewChat::on_createButton_clicked()
{
    QString title = ui->titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Chat title cannot be empty");
        return;
    }

    // Get selected users
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

    // Here you would typically send this data to the server
    // For now, just emit the signal
    emit chatCreated(title);
    accept();
}
