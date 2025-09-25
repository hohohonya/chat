#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "dialog.h"
#include "new_chat.h"
#include "RemoveDialog.h"
#include "adddialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QTimer>
#include <QColor>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupSocket();


    ui->new_chat->setEnabled(false);
    ui->send->setEnabled(false);
    ui->add->setEnabled(false);
    ui->remove->setEnabled(false);
    ui->leave_chat->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSocket()
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::connectedToServer);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::disconnectedFromServer);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
    //socket->connectToHost("localhost", 1234);
    socket->connectToHost("26.13.49.181", 1234);
    //socket->connectToHost("26.200.42.39", 1234);
}

void MainWindow::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}

void MainWindow::on_action_triggered()
{
    loginDialog = new Dialog(this);

    //loginDialog->setSocket(socket);



    connect(loginDialog, &Dialog::loginSuccess, this, &MainWindow::handleLoginSuccess);
    connect(loginDialog, &Dialog::registrationSuccess, this, &MainWindow::handleRegistrationSuccess);

    loginDialog->exec();
}

void MainWindow::on_new_chat_clicked()
{
    newChatDialog = new New_chat(this);
    newChatDialog->setCurrentUserId(currentUsername);
    connect(newChatDialog, &New_chat::chatCreated, this, &MainWindow::handleChatCreated);
    QJsonObject request;
    request["action"] = "getAllUsers";
    sendRequest(request);

    newChatDialog->exec();
}
void MainWindow::on_remove_clicked()
{
    // QJsonObject request;
    // request["action"] = "isUserAdmin";
    // request["username"] = currentUsername;
    // sendRequest(request);
    qDebug() << isAdmin;


    if(!isAdmin){
        QMessageBox::warning(this,"Error", "You dont have such premison");
    }
    else{
        removeDialog = new RemoveDialog(this);
        removeDialog->setCurrentChatTitle(currentChatTitle);
        QJsonObject request;
        request["action"] = "getChatUsers";
        request["chatTitle"] = currentChatTitle;
        sendRequest(request);
        connect(removeDialog, &RemoveDialog::userRemoved, this, &MainWindow::handleUserRemovedFromChat);
        removeDialog->exec();
    }

    // removeDialog = new RemoveDialog(this);
    // removeDialog->setCurrentChatTitle(currentChatTitle);
    // QJsonObject request;
    // request["action"] = "getChatUsers";
    // request["chatTitle"] = currentChatTitle;
    // sendRequest(request);
    // connect(removeDialog, &RemoveDialog::userRemoved, this, &MainWindow::handleUserRemovedFromChat);
    // removeDialog->exec();
}
void MainWindow::on_add_clicked()
{
    addDialog = new AddDialog(this);
    addDialog->setCurrentChatTitle(currentChatTitle);
    QJsonObject request;
    request["action"] = "getNotChatUsers";
    request["chatTitle"] = currentChatTitle;
    sendRequest(request);
    connect(addDialog, &AddDialog::userAdded, this, &MainWindow::handleUserAddedToChat);
    addDialog->exec();
}

void MainWindow::on_send_clicked()
{
    QString message = ui->textEdit->toPlainText().trimmed();
    if (message.isEmpty() || currentChatTitle.isEmpty()) return;

    QJsonObject request;
    request["action"] = "sendMessage";
    request["chatTitle"] = currentChatTitle;
    request["username"] = currentUsername;
    request["message"] = message;
    sendRequest(request);
    QJsonObject updateRequest;
    updateRequest["action"] = "getChatMessages";
    updateRequest["chatTitle"] = currentChatTitle;
    sendRequest(updateRequest);

    ui->textEdit->clear();
}

void MainWindow::on_listW_clicked(const QModelIndex &index)
{
    currentChatTitle = index.data().toString();
    ui->title->setText("Chat: " + currentChatTitle);

    QJsonObject request;
    request["action"] = "getChatMessages";
    request["chatTitle"] = currentChatTitle;
    sendRequest(request);
    request["action"] = "isUserAdmin";
    request["username"] = currentUsername;
    sendRequest(request);

    qDebug() << isAdmin;
}

void MainWindow::connectedToServer()
{
    ui->statusbar->showMessage("Connected to server", 3000);
}

void MainWindow::disconnectedFromServer()
{
    ui->statusbar->showMessage("Disconnected from server", 3000);
}

void MainWindow::readyRead()
{
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject response = doc.object();
    QString action = response["action"].toString();

    if (action == "authenticate" && response["success"].toBool()) {
        emit handleLoginSuccess(response["username"].toString());
    }
     if (action == "register" && response["success"].toBool()) {
        emit handleRegistrationSuccess();
    }
    else if (action == "getUserChats") {
        QJsonArray chats = response["chats"].toArray();
        ui->listW->clear();
        for (const auto &chat : chats) {
            QJsonObject chatObj = chat.toObject();
            QListWidgetItem* item = new QListWidgetItem(chatObj["title"].toString());
            item->setBackground(QColor("white"));
            item->setForeground(QColor("black"));
            ui->listW->addItem(item);
        }
    }
    else if (action == "getChatMessages") {
        QJsonArray messages = response["messages"].toArray();
        displayMessages(messages);
    }
     else if (action == "createChat" && response["success"].toBool()) {
        QString title = response["title"].toString();
        QString admin = response["admin"].toString();
        QJsonObject request;
        request["action"] = "getUserChats";
        request["username"] = currentUsername;
        sendRequest(request);
        QMessageBox::information(this, "Success", "Chat created successfully!");
     }
    else if (action == "getAllUsers") {
        QJsonArray users = response["users"].toArray();
        QStringList userList;
        for (const auto &user : users) {
            userList << user.toString();
        }

        if (newChatDialog && newChatDialog->isVisible()) {
            newChatDialog->setUserList(userList);
        }
    }
     else if (action == "getChatUsers") {
         QJsonArray users = response["users"].toArray();
         QStringList userList;
         for (const auto &user : users) {
            if(user != currentUsername)
                userList << user.toString();
         }
         if (removeDialog && removeDialog->isVisible()) {
             removeDialog->setUserList(userList);
         }
    }
     else if (action == "getNotChatUsers") {
         QJsonArray users = response["users"].toArray();
         QStringList userList;
         for (const auto &user : users) {
             userList << user.toString();
         }
         if (addDialog && addDialog->isVisible()) {
             addDialog->setUserList(userList);
         }
    }
     else if (action == "isUserAdmin") {
             isAdmin = response["isAdmin"].toBool();
     }
     else if (action == "removeUserFromChat") {
         if (response["success"].toBool()) {
             QMessageBox::information(this, "Success", "Users removed successfully");
             if (removeDialog && removeDialog->isVisible()) {
                 QJsonObject request;
                 request["action"] = "getChatUsers";
                 request["chatTitle"] = currentChatTitle;
                 sendRequest(request);
             }
         } else {
             QMessageBox::warning(this, "Error", "Failed to remove users");
         }
     }
     else if (action == "addUserToChat") {
         if (response["success"].toBool()) {
             QMessageBox::information(this, "Success", "Users added successfully");
             if (addDialog && addDialog->isVisible()) {
                 QJsonObject request;
                 request["action"] = "getChatUsers";
                 request["chatTitle"] = currentChatTitle;
                 sendRequest(request);
             }
         } else {
             QMessageBox::warning(this, "Error", "Failed to add users");
         }
     }
     else if (action == "updateMessages") {
         QString chatTitle = response["chatTitle"].toString();
         QJsonArray messages = response["messages"].toArray();

         if (currentChatTitle == chatTitle) {
             displayMessages(messages);
         }
     }
     else if (action == "updateUserChats") {
        if (response["username"].toString() == currentUsername) {

            if (response["chatTitle"].toString() == currentChatTitle){
                currentChatTitle = NULL;
                qDebug() << currentChatTitle.isEmpty();
                ui->listW->clear();
                ui->listWidget->clear();
                ui->title->clear();
            }

            updateChatList();
        }

     }
}

void MainWindow::handleLoginSuccess(const QString &username)
{
    currentUsername = username;
    ui->new_chat->setEnabled(true);
    ui->send->setEnabled(true);
    ui->add->setEnabled(true);
    ui->remove->setEnabled(true);
    ui->leave_chat->setEnabled(true);

    ui->connected->setText("Connected as: " + username);

    QJsonObject request;
    request["action"] = "getUserChats";
    request["username"] = currentUsername;
    sendRequest(request);
}

void MainWindow::handleRegistrationSuccess()
{
    QMessageBox::information(this, "Registration", "Registration successful!");
}

void MainWindow::handleChatCreated(const QString &title,const QString &currentUser, const QStringList &selectedUsers)
{
    QMessageBox::information(this, "New Chat", "Chat '" + title + "' created successfully!");
    updateChatList();
}

void MainWindow::handleUserAddedToChat()
{
    QMessageBox::information(this, "User Added", "User added to chat successfully!");
    updateChatList();
}

void MainWindow::handleUserRemovedFromChat()
{
    QMessageBox::information(this, "User Removed", "User removed from chat successfully!");
    updateChatList();
}

void MainWindow::updateChatList()
{
    QJsonObject request;
    request["action"] = "getUserChats";
    request["username"] = currentUsername;
    sendRequest(request);
}

void MainWindow::displayMessages(const QJsonArray &messages)
{
    qDebug() << "Received messages:" << messages;
    ui->listWidget->clear();

    for (const auto &message : messages) {
        QJsonObject messageObj = message.toObject();
        QString username = messageObj["id"].toString();
        QString date = messageObj["date"].toString();
        QString text = messageObj["text"].toString();

        date = date.left(19);

        QString displayName = (username == currentUsername) ? currentUsername + " (Me)" : username;

        QString messageText = QString("%2 • %3\n%1").arg(text, date, displayName);

        QListWidgetItem *item = new QListWidgetItem;
        item->setText(messageText);
        item->setBackground(QColor(200, 200, 255));
        item->setForeground(QColor("black"));


        if (username == currentUsername) {
            item->setTextAlignment(Qt::AlignRight);
        } else {
            item->setTextAlignment(Qt::AlignLeft);
        }

        ui->listWidget->addItem(item);
    }

    ui->listWidget->scrollToBottom();
}

void MainWindow::on_leave_chat_clicked()
{
    QJsonObject request;
    request["action"] = "userLeaveChat";
    request["chatTitle"] = currentChatTitle;
    request["username"] = currentUsername;
    sendRequest(request);

    currentChatTitle = NULL;
    qDebug() << currentChatTitle.isEmpty();
    ui->listW->clear();
    ui->listWidget->clear();
    ui->title->clear();

    updateChatList();
}

