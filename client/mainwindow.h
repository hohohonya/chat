#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Dialog;
class New_chat;
class RemoveDialog;
class AddDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void chatCreationFailed(const QString &title, const QString &error);

private slots:
    void on_action_triggered();
    void on_new_chat_clicked();
    void on_add_clicked();
    void on_remove_clicked();
    void on_send_clicked();
    void on_listW_clicked(const QModelIndex &index);

    void connectedToServer();
    void disconnectedFromServer();
    void readyRead();

    void handleLoginSuccess(const QString &username);
    void handleRegistrationSuccess();
    void handleChatCreated(const QString &title,const QString &currentUser, const QStringList &selectedUsers);
    void handleUserAddedToChat();
    void handleUserRemovedFromChat();

    void on_leave_chat_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString currentUsername;
    QString currentChatTitle;
    QString admin;
    bool isAdmin;

    Dialog *loginDialog;
    New_chat *newChatDialog;
    RemoveDialog *removeDialog;
    AddDialog *addDialog;

    void sendRequest(const QJsonObject &request);
    void updateChatList();
    void displayMessages(const QJsonArray &messages);
    void setupSocket();
};

#endif // MAINWINDOW_H
