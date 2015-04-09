#ifndef SPREADMANAGER_H
#define SPREADMANAGER_H

#include <QObject>
#include <QtCore>
#include "sp.h"

#define MAX_MESSLEN     102400
#define MAX_VSSETS      10
#define MAX_MEMBERS     100

class SpreadManager : public QObject
{
    Q_OBJECT

public:
    explicit SpreadManager(QObject *parent = 0);
    ~SpreadManager();
    void initConnection(QString _port, QString _name, QString _group);
    void Bye();
    bool connected = false;

private:

    char *group_name;
    mailbox Mbox;
    char Private_group[MAX_GROUP_NAME];
    int     To_exit = 0;

    char* toChar(QString str);
    void Read_thread_routine();
    void Read_message(int fd, int code, void *data);

signals:
    void didConnect();
    void messReceived(char* mess);

public slots:
    void sendMes(QString mess);

};

#endif // SPREADMANAGER_H
