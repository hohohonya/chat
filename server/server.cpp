#include "server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


Server::Server(QObject *parent): QTcpServer(parent)
{
    if(!db.connectToDatabase()){
        qDebug()<<"Failed to connect to database";
    }
}

void Server::startServer()
{
    if (this->listen(QHostAddress::Any, 1234)){
        qDebug()<<"Server started";
    } else{
        qDebug()<<"Server failed to start";
    }
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);
    clients.append(socket);
    qDebug()<<"Client connected:"<<clients.size();
}

void Server::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    buffer += socket->readAll();

    while (!buffer.isEmpty()) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(buffer, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            if (doc.isObject()) {
                processRequest(socket, doc.toJson());
                buffer.remove(0, doc.toJson().length());
            } else {
                qDebug() << "Received data is not a JSON object";
                buffer.clear();
            }
        } else if (parseError.error == QJsonParseError::GarbageAtEnd) {
            int endPos = buffer.indexOf("}\n{") + 1;
            if (endPos <= 0) break;

            QByteArray firstJson = buffer.left(endPos + 1);
            doc = QJsonDocument::fromJson(firstJson, &parseError);
            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                processRequest(socket, firstJson);
                buffer.remove(0, firstJson.length());
            } else {
                qDebug() << "Failed to parse partial JSON";
                break;
            }
        } else {
            qDebug() << "JSON parse error:" << parseError.errorString();
            buffer.clear();
            break;
        }
    }
}

void Server::disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    int num=clients.indexOf(socket)+1;
    clients.removeAll(socket);
    socket->deleteLater();
    qDebug() << "Client disconnected"<<num;
}

void Server::processRequest(QTcpSocket *socket, const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON received";
        return;
    }
    QJsonObject request = doc.object();
    QString action = request["action"].toString();
    QJsonObject response;
    response["action"] = action;


    if (action == "authenticate") {
        QString username = request["username"].toString();
        QString password = request["password"].toString();

        bool success = db.authenticateUser(username, password);
        response["success"] = success;
        if (success) {
            response["username"] = username;
        }
    }
    else if (action == "register") {
        //QString name = request["name"].toString();
        //QString surname = request["surname"].toString();
        QString username = request["username"].toString();
        QString password = request["password"].toString();

        //name, surname,
        bool success = db.registerUser(username, password);
        response["success"] = success;
    }
    else if (action == "getUserChats") {
        QString username = request["username"].toString();
        auto chats = db.getUserChats(username);

        QJsonArray chatArray;
        for (const auto &chat : chats) {
            QJsonObject chatObj;
            chatObj["title"] = chat.first;
            chatObj["id"] = chat.second;
            chatArray.append(chatObj);
        }
        response["chats"] = chatArray;
    }
    else if (action == "getChatMessages") {
        QString chatTitle = request["chatTitle"].toString();
        auto messages = db.getChatMessages(chatTitle);

        QJsonArray messageArray;
        for (const auto &message : messages) {
            QJsonObject messageObj;
            messageObj["id"] = message[0];
            messageObj["date"] = message[1];
            messageObj["text"] = message[2];
            messageArray.append(messageObj);
        }
        response["messages"] = messageArray;
    }
    else if (action == "createChat") {
        QString title = request["title"].toString();
        QString admin = request["admin"].toString();

        QStringList users;
        if (request.contains("users") && request["users"].isArray()) {
            QJsonArray usersArray = request["users"].toArray();
            for (const QJsonValue &user : usersArray) {
                users.append(user.toString());
            }
        }
        if (!users.contains(admin)) {
            users.append(admin);
        }
        bool success = db.createChat(title, admin, users);
        response["success"] = success;
        if (success) {
             QStringList chatUsers = db.getChatUsers(title);
             for (const QString &user: chatUsers)
                {
                QJsonObject updateRequest;
                updateRequest["action"] = "updateUserChats";
                updateRequest["username"] = user;

                for (QTcpSocket *client : clients) {
                    sendResponse(client, updateRequest);
                }
            }
        }

    }
    else if (action == "addUserToChat") {
        QString chatTitle = request["chatTitle"].toString();
        QString username = request["username"].toString();
        QString admin = request["admin"].toString();

        bool success = db.addUserToChat(chatTitle, username, admin);
        response["success"] = success;
    }
    else if (action == "addUsersToChat") {
        QString chatTitle = request["chatTitle"].toString();
        QStringList users;
        if (request.contains("users") && request["users"].isArray()) {
            QJsonArray usersArray = request["users"].toArray();
            for (const QJsonValue &user : usersArray) {
                users.append(user.toString());
            }
        }
        bool success = db.addUsersToChat(chatTitle,users);
        response["success"] = success;
        if (success) {
            QStringList affectedUsers = db.getChatUsers(chatTitle);
            for (const QString &user : affectedUsers) {
                QJsonObject updateRequest;
                updateRequest["action"] = "updateUserChats";
                updateRequest["username"] = user;

                for (QTcpSocket *client : clients) {
                    sendResponse(client, updateRequest);
                }
            }
        }
    }
    else if (action == "removeUserFromChat") {
        QString chatTitle = request["chatTitle"].toString();
        QStringList users;
        if (request.contains("users") && request["users"].isArray()) {
            QJsonArray usersArray = request["users"].toArray();
            for (const QJsonValue &user : usersArray) {
                users.append(user.toString());
            }
        }
        bool success = db.removeUsersFromChat(chatTitle,users);
        response["success"] = success;
        if (success) {
            QStringList affectedUsers = db.getChatUsers(chatTitle);
            for (const QString &user : affectedUsers) {
                QJsonObject updateRequest;
                updateRequest["action"] = "updateUserChats";
                updateRequest["username"] = user;

                for (QTcpSocket *client : clients) {
                    sendResponse(client, updateRequest);
                }
            }
            for (const QString &removedUser : users) {
                QJsonObject updateRequest;
                updateRequest["action"] = "updateUserChats";
                updateRequest["username"] = removedUser;
                updateRequest["chatTitle"] = chatTitle;

                for (QTcpSocket *client : clients) {
                    sendResponse(client, updateRequest);
                }
            }
        }
    }
    else if (action == "sendMessage") {
        QString chatTitle = request["chatTitle"].toString();
        QString username = request["username"].toString();
        QString message = request["message"].toString();

        bool success = db.sendMessage(chatTitle, username, message);
        response["success"] = success;

        if (success) {
            QStringList chatUsers = db.getChatUsers(chatTitle);

            auto messages = db.getChatMessages(chatTitle);

            QJsonObject updateMessage;
            updateMessage["action"] = "updateMessages";
            updateMessage["chatTitle"] = chatTitle;

            QJsonArray messageArray;
            for (const auto &msg : messages) {
                QJsonObject messageObj;
                messageObj["id"] = msg[0];
                messageObj["date"] = msg[1];
                messageObj["text"] = msg[2];
                messageArray.append(messageObj);
            }
            updateMessage["messages"] = messageArray;

            for (QTcpSocket *client : clients) {
                sendResponse(client, updateMessage);
            }
        }
    }
    else if (action == "getAllUsers") {
        auto users = db.getAllUsers();

        QJsonArray userArray;
        for (const auto &user : users) {
            userArray.append(user);
        }
        response["users"] = userArray;
    }
    else if (action == "getChatUsers") {
        QString chatTitle = request["chatTitle"].toString();
        auto users = db.getChatUsers(chatTitle);

        QJsonArray usersArray;
        for (const auto &user : users) {
            usersArray.append(user);
        }
        response["users"] = usersArray;
    }
    else if (action == "getNotChatUsers") {
        QString chatTitle = request["chatTitle"].toString();
        auto users = db.getNotChatUsers(chatTitle);

        QJsonArray usersArray;
        for (const auto &user : users) {
            usersArray.append(user);
        }
        response["users"] = usersArray;
    }
    else if (action == "isUserAdmin") {
            QString chatTitle = request["chatTitle"].toString();
            QString username = request["username"].toString();

            bool isAdmin = db.isUserAdmin(chatTitle, username);
            response["isAdmin"] = isAdmin;
    }
    else if (action == "userLeaveChat"){
        QString chatTitle = request["chatTitle"].toString();
        QString username = request["username"].toString();
        db.leaveChat(chatTitle, username);

    }
    else {
        response["error"] = "Unknown action";
    }

    sendResponse(socket, response);
}

void Server::sendResponse(QTcpSocket *socket, const QJsonObject &response)
{
    QJsonDocument doc(response);
    socket->write(doc.toJson(QJsonDocument::Compact));
    socket->write("\n");
    socket->flush();
}

