#ifndef SERVER_H
#define SERVER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>
#include <QVector>
#include "database.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void startServer();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readyRead();
    void disconnected();

private:
    QVector<QTcpSocket*> clients;
    Database db;
    void processRequest(QTcpSocket *socket, const QByteArray &data);
    void sendResponse(QTcpSocket *socket, const QJsonObject &response);
    QByteArray buffer;
};
#endif // SERVER_H
