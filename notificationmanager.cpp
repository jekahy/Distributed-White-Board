#include "notificationmanager.h"

NotificationManager::NotificationManager(QObject *parent) : QObject(parent)
{

}

NotificationManager::~NotificationManager()
{

}

void NotificationManager::showAlert(std::function<void(QMessageBox*)>& btnFunc){

    QMessageBox *msgBox = new QMessageBox();
    btnFunc(msgBox);
    msgBox->setWindowTitle("Attention");
    msgBox->exec();
}
