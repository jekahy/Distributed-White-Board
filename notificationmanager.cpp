#include "notificationmanager.h"

NotificationManager::NotificationManager(QObject *parent) : QObject(parent)
{

}

NotificationManager::~NotificationManager()
{

}

void NotificationManager::showAlert(QString mess, std::function< void(int) >& lambda){

//void NotificationManager::showAlert(QString mess, std::function< void(QMessageBox) > &butFunc, std::function< void(int) >& lambda){

    QMessageBox msgBox;
//    QMessageBox::StandardButton reply;
//    msgBox.setStandardButtons();
    msgBox.setWindowTitle("Attention");
    msgBox.setText(mess);
//    butFunc(&msgBox);
//    msgBox.setInformativeText("Do you want to save your changes?");
//    msgBox.setStandardButtons(QMessageBox::Ok);
//    msgBox.setStandardButtons(QMessageBox::standardButtons());
    msgBox.setDefaultButton(QMessageBox::Ok);
//    int ret = ;
    lambda(msgBox.exec());
}
