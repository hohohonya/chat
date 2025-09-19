#include "registration.h"
#include "ui_registration.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

registration::registration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::registration)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    //socket->connectToHost("localhost", 1234);
    //socket->connectToHost("26.200.42.39", 1234);
    socket->connectToHost("26.13.49.181", 1234);
}

registration::~registration()
{
    delete ui;
}
void registration::sendRequest(const QJsonObject &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QJsonDocument doc(request);
        socket->write(doc.toJson());
    } else {
        QMessageBox::warning(this, "Connection Error", "Not connected to server");
    }
}

void registration::on_registerButton_clicked()
{
    //QString name = ui->nameEdit->text().trimmed();
    //QString surname = ui->surnameEdit->text().trimmed();
    QString username = ui->idEdit->text().trimmed();
    QString password = ui->passwEdit->text().trimmed();
    QString confirmPassword = ui->passEdit->text().trimmed();


    //name.isEmpty() || surname.isEmpty() ||
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "All fields are required");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Input Error", "Passwords do not match");
        return;
    }
    QJsonObject request;
    request["action"] = "register";
    //request["name"] = name;
    //request["surname"] = surname;
    request["username"] = username;
    request["password"] = password;

    sendRequest(request);


    if (socket->waitForReadyRead(3000)) {
        QByteArray data = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;

        QJsonObject response = doc.object();
        if (response["action"] == "register" && response["success"].toBool()) {
            emit registrationSuccess();
            accept();
        } else {
            QMessageBox::warning(this, "Registration Failed", "Username already exists");
        }
    } else {
        QMessageBox::warning(this, "Timeout", "No response from server");
    }
}
