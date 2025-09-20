#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
    void setSocket(QTcpSocket *soc);

signals:
    void loginSuccess(const QString &username);
    void registrationSuccess();

private slots:
    void on_sign_clicked();
    void on_create_clicked();

private:
    Ui::Dialog *ui;
    QTcpSocket *socket;
    void sendRequest(const QJsonObject &request);
};

#endif // DIALOG_H
