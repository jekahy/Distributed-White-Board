#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QMessageBox>
#include "singleton.h"

class NotificationManager : public QObject
{
    Q_OBJECT
public:
    explicit NotificationManager(QObject *parent = 0);
    ~NotificationManager();

    static void showAlert(std::function<void(QMessageBox*)> &btnFunc);

};

#endif // NOTIFICATIONMANAGER_H
