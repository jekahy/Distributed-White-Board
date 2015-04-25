#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QMessageBox>
#include "singleton.h"

//template<typename Functor>

class NotificationManager : public QObject
{
    Q_OBJECT
public:
    explicit NotificationManager(QObject *parent = 0);
    ~NotificationManager();

//    void f(std::function< int(int) >& lambda);
//    void showAlert(Functor butFunc, std::function< void(int) >& lambda);
    void showAlert(QString mess, std::function< void(int) >& lambda);

private:


signals:

public slots:

};

#endif // NOTIFICATIONMANAGER_H
