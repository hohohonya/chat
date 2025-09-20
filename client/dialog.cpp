#include "dialog.h"
#include "ui_dialog.h"
#include "registration.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);


    //socket->connectToHost("localhost", 1234);
    //socket->connectToHost("26.200.42.39", 1234);


    socket->connectToHost("26.13.49.181", 1234);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setSocket(QTcpSocket *soc)
{
    socket = soc;
}



// Функция для преобразования состояния в строку
QString socketStateToString(QAbstractSocket::SocketState state) {
    switch(state) {
        case QAbstractSocket::UnconnectedState: return "Unconnected";
        case QAbstractSocket::HostLookupState: return "HostLookup";
        case QAbstractSocket::ConnectingState: return "Connecting";
        case QAbstractSocket::ConnectedState: return "Connected";
        case QAbstractSocket::BoundState: return "Bound";
        case QAbstractSocket::ListeningState: return "Listening";
        case QAbstractSocket::ClosingState: return "Closing";
        default: return "Unknown";
    }
}







void Dialog::sendRequest(const QJsonObject &request)
{
    qDebug() << socketStateToString(socket->state());
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}

void Dialog::on_sign_clicked()
{
    QString username = ui->user->text().trimmed();
    QString password = ui->pass->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Username and password cannot be empty");
        return;
    }

    QJsonObject request;
    request["action"] = "authenticate";
    request["username"] = username;
    request["password"] = password;
    sendRequest(request);

    if (socket->waitForReadyRead(3000)) {
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;

        QJsonObject response = doc.object();
        if (response["action"] == "authenticate" && response["success"].toBool()) {
            emit loginSuccess(username);
            //this->socket;
            this->close();
        } else {
            QMessageBox::warning(this, "Login Failed", "Invalid username or password");
        }
    } else {
        QMessageBox::warning(this, "Timeout", "No response from server");
    }
}

void Dialog::on_create_clicked()
{
    registration *regDialog = new registration(this);
    connect(regDialog, &registration::registrationSuccess, this, &Dialog::registrationSuccess);
    regDialog->exec();
}
