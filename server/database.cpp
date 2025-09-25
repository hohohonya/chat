#include "database.h"

Database::Database(QObject *parent): QObject(parent)
{
    db=QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("osinew");
    db.setPort(5432);
    db.setUserName("postgres");
    db.setPassword("12345");
}

bool Database::connectToDatabase()
{
    if (!db.open()){
        qDebug()<<"Error connecting to database:"<<db.lastError().text();
        return false;
    }
    return true;
}

bool Database::authenticateUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE id=:id");
    query.bindValue(":id",username);
    if (!query.exec()){
        qDebug()<<"Authentication error:"<<query.lastError().text();
        return false;
    }
    if(query.next()){
        QString storedPassword = query.value(0).toString();
        return storedPassword==password;
    }
    return false;
}

//const QString &name, const QString &surname,
bool Database::registerUser(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users(id,name, surname,password) VALUES (:id, :name, :surname, :password)");
    query.bindValue(":id", username);
    query.bindValue(":name", '...');
    query.bindValue(":surname", '...');
    query.bindValue(":password", password);

    if(!query.exec()){
        qDebug()<<"Registration error:"<<query.lastError().text();
        return false;
    }
    return true;
}

QVector<QPair<QString, QString> > Database::getUserChats(const QString &username)
{
    QVector<QPair<QString, QString>> chats;
    QSqlQuery query;
    query.prepare("SELECT chat_title, user_id FROM chat_members WHERE user_id=:id");
    query.bindValue(":id", username);

    if(query.exec()){
        while (query.next()){
            chats.append(qMakePair(query.value(0).toString(), query.value(1).toString()));
        }
    } else{
        qDebug() << "Error getting user chats:" << query.lastError().text();
    }
    return chats;
}

QVector<QVector<QString> > Database::getChatMessages(const QString &chatTitle)
{
    QVector<QVector<QString>> messages;
    QSqlQuery query;
    query.prepare("SELECT id, date, text FROM messages WHERE title = :title ORDER BY date");
    query.bindValue(":title", chatTitle);

    if(query.exec()){
        while (query.next()){
            QVector<QString> message;
            message.append(query.value(0).toString());
            message.append(query.value(1).toString());
            message.append(query.value(2).toString());
            messages.append(message);
        }
    } else{
        qDebug() << "Error getting user chats:" << query.lastError().text();
    }
    return messages;
}

bool Database::createChat(const QString &title, const QString &admin, const QStringList &users)
{
    if (!db.isOpen() && !connectToDatabase()) {
        return false;
    }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT title FROM chats WHERE title = :title LIMIT 1");
    checkQuery.bindValue(":title", title);
    if (checkQuery.exec() && checkQuery.next()) {
        qDebug() << "Chat with this title already exists";
        return false;
    }
    db.transaction();

    try {
        QSqlQuery createQuery;
                createQuery.prepare("INSERT INTO chats (title, admin) VALUES (:title, :admin)");
                createQuery.bindValue(":title", title);
                createQuery.bindValue(":admin", admin);
                if (!createQuery.exec()) {
                    db.rollback();
                    qDebug() << "Error creating chat:" << createQuery.lastError().text();
                    return false;
                }

                for (const QString &user : users) {
                    QSqlQuery memberQuery;
                    memberQuery.prepare("INSERT INTO chat_members (chat_title, user_id) VALUES (:title, :user)");
                    memberQuery.bindValue(":title", title);
                    memberQuery.bindValue(":user", user);
                    if (!memberQuery.exec()) {
                        db.rollback();
                        qDebug() << "Error adding user to chat:" << memberQuery.lastError().text();
                        return false;
                    }
                }


                if (!users.contains(admin)) {
                    QSqlQuery adminQuery;
                    adminQuery.prepare("INSERT INTO chat_members (chat_title, user_id) VALUES (:title, :user)");
                    adminQuery.bindValue(":title", title);
                    adminQuery.bindValue(":user", admin);
                    if (!adminQuery.exec()) {
                        db.rollback();
                        qDebug() << "Error adding admin to chat:" << adminQuery.lastError().text();
                        return false;
                    }
                }

                if (!db.commit()) {
                    qDebug() << "Commit failed:" << db.lastError().text();
                    return false;
                }
                return true;

            } catch (...) {
                db.rollback();
                return false;
            }
    }



bool Database::addUserToChat(const QString &chatTitle, const QString &username,const QString &admin)
{
    QSqlQuery query;
    query.prepare("INSERT INTO chat_members (chat_title, user_id) VALUES (:title, :id)");
    query.bindValue(":title", chatTitle);
    query.bindValue(":id", username);

    if (!query.exec()) {
        qDebug() << "Error adding user to chat:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::removeUserFromChat(const QString &chatTitle, const QString &username)
{
    QSqlQuery query;
    query.prepare("DELETE FROM chat_members WHERE chat_title = :title AND user_id = :id");
    query.bindValue(":title", chatTitle);
    query.bindValue(":id", username);

    if (!query.exec()) {
        qDebug() << "Error removing user from chat:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::removeUsersFromChat(const QString &chatTitle, const QStringList &users)
{
    QSqlQuery query;
    query.prepare("SELECT admin FROM chats WHERE title = :title");
    query.bindValue(":title", chatTitle);

    if (!query.exec() || !query.next()) {
            qDebug() << "Failed to get chat admin:" << query.lastError().text();
            return false;
        }

    QString admin = query.value(0).toString();
    db.transaction();

    for (const QString &user : users) {
        if (user == admin) continue;
        if (!removeUserFromChat(chatTitle, user)) {
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        qDebug() << "Commit failed:" << db.lastError().text();
        return false;
    }
    return true;
}












bool Database::addUsersToChat(const QString &chatTitle, const QStringList &users)
{
    QSqlQuery query;
    query.prepare("SELECT admin FROM chats WHERE title = :title");
    query.bindValue(":title", chatTitle);

    if (!query.exec() || !query.next()) {
            qDebug() << "Failed to get chat admin:" << query.lastError().text();
            return false;
        }

    QString admin = query.value(0).toString();
    db.transaction();

    for (const QString &user : users) {
        if (!addUserToChat(chatTitle, user, admin)) {
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        qDebug() << "Commit failed:" << db.lastError().text();
        return false;
    }
    return true;
}
bool Database::sendMessage(const QString &chatTitle, const QString &username, const QString &message)
{
    QSqlQuery query;

    query.prepare("INSERT INTO messages (id, text, title) VALUES (:id, :text, :title)");
    query.bindValue(":id", username);
    query.bindValue(":text", message);
    query.bindValue(":title", chatTitle);

    if (!query.exec()) {
        qDebug() << "Error sending message:" << query.lastError().text();
        return false;
    }

    return true;
}

QVector<QString> Database::getAllUsers()
{
    QVector<QString> users;
    QSqlQuery query("SELECT id FROM users");

    while (query.next()) {
        users.append(query.value(0).toString());
    }

    return users;
}

QStringList Database::getChatUsers(const QString &chatTitle)
{
    QStringList users;
    QSqlQuery query;
    query.prepare("SELECT user_id FROM chat_members WHERE chat_title = :title");
    query.bindValue(":title", chatTitle);

    if (query.exec()) {
        while (query.next()) {
            users.append(query.value(0).toString());
        }
    } else {
        qDebug() << "Failed to get chat users:" << query.lastError();
    }

    return users;
}

QStringList Database::getNotChatUsers(const QString &chatTitle)
{
    QStringList users;
    QSqlQuery query;
    query.prepare("SELECT u.id FROM users u WHERE u.id NOT IN (SELECT c.user_id FROM chat_members c WHERE c.chat_title = :title)");
    query.bindValue(":title", chatTitle);

    if (query.exec()) {
        while (query.next()) {
            users.append(query.value(0).toString());
        }
    } else {
        qDebug() << "Failed to get chat users:" << query.lastError();
    }

    return users;
}
// bool Database::isUserAdmin(const QString &chatTitle, const QString &username)
// {
//     QSqlQuery query;
//     query.prepare("SELECT admin FROM chats WHERE title = :title AND id = :id");
//     query.bindValue(":title", chatTitle);
//     query.bindValue(":id", username);

//     if (query.exec() && query.next()) {
//         return query.value(0).toBool();
//     }

//     return false;
// }

//}
bool Database::isUserAdmin(const QString &chatTitle, const QString &username)
{
    QSqlQuery query;
    query.prepare("SELECT admin FROM chats WHERE title = :title AND admin = :id");
    query.bindValue(":title", chatTitle);
    query.bindValue(":id", username);

    if (query.exec() && query.next()) {
        return query.value(0).toBool();
    }

    return false;
}



// bool Database::leaveChat(const QString &chatTitle, const QString &username)
// {
//     if (isUserAdmin(chatTitle, username))
//     {
//         qDebug() << "We are at start";
//         QSqlQuery query;
//         query.prepare("SELECT user_id from chat_members WHERE chat_title = :title ORDER BY user_id LIMIT(1)");

//         qDebug() << "first step";

//         query.bindValue(":title", chatTitle);
//         QString new_admin_id = query.value(0).toString();

//         qDebug() << "second step";

//         query.clear();
//         if (!query.exec()) {
//             qDebug() << "Error removing user from chat:" << query.lastError().text();
//             return false;
//         }

//         qDebug() << "We are in center";
//         query.prepare("UPDATE chats SET admin = :new WHERE title = :title");
//         query.bindValue(":new", new_admin_id);
//         query.bindValue(":title", chatTitle);
//         if (!query.exec()) {
//             qDebug() << "Error removing user from chat:" << query.lastError().text();
//             return false;
//         }

//         removeUserFromChat(chatTitle, username);
//     }
//     else
//     {
//         removeUserFromChat(chatTitle, username);
//     }

//     return true;
// }


bool Database::leaveChat(const QString &chatTitle, const QString &username)
{
    if (isUserAdmin(chatTitle, username))
    {
        QSqlQuery query;
        query.prepare("SELECT user_id FROM chat_members WHERE chat_title = :title AND user_id <> :id ORDER BY user_id LIMIT 1");
        query.bindValue(":title", chatTitle);
        query.bindValue(":id", username);

        // Выполняем запрос и проверяем ошибки
        if (!query.exec()) {
            qDebug() << "Error selecting new admin:" << query.lastError().text();
            return false;
        }

        QString new_admin_id;
        if (query.next()) {
            new_admin_id = query.value(0).toString();
        } else {
            // Обработка случая, когда в чате нет других участников
            qDebug() << "No other members in chat";
            return false;
        }

        query.prepare("UPDATE chats SET admin = :new WHERE title = :title");
        query.bindValue(":new", new_admin_id);
        query.bindValue(":title", chatTitle);

        if (!query.exec()) {
            qDebug() << "Error updating chat admin:" << query.lastError().text();
            return false;
        }
    }

    // Удаляем пользователя из чата в любом случае
    if (!removeUserFromChat(chatTitle, username)) {
        qDebug() << "Error removing user from chat";
        return false;
    }

    return true;
}

