#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class registration;
}

class registration : public QDialog
{
    Q_OBJECT

public:
    explicit registration(QWidget *parent = nullptr);
    ~registration();

signals:
    void registrationSuccess();

private slots:
    void on_registerButton_clicked();

private:
    Ui::registration *ui;
    QTcpSocket *socket;
    void sendRequest(const QJsonObject &request);
};
#endif // REGISTRATION_H
