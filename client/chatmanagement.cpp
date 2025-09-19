#include "chatmanagement.h"
#include "ui_chatmanagement.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QCheckBox>


chatmanagement::chatmanagement(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chatmanagement),
    chatTitle(chatTitle),
    currentUser(currentUser),
    admin(Admin)
{
    ui->setupUi(this);
}

chatmanagement::~chatmanagement()
{
    delete ui;
}
