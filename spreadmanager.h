#ifndef SPREADMANAGER_H
#define SPREADMANAGER_H

#include <QObject>
#include <QtCore>
#include "sp.h"
#include <atomic>
#include <QtGui>

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
    void closeConnection();
    void Bye();
    volatile std::atomic<bool> connected;

    void startDrawing(QPoint p);
    void continueDrawing(QPoint p);
    void stopDrawing(QPoint p);

    void sendMes(QString mess);

private:

    QMutex q_mutex;
    char *group_name;
    mailbox Mbox;
    char Private_group[MAX_GROUP_NAME];
    int     To_exit = 0;

    char* toChar(QString str);
    void Read_thread_routine();
    void Read_message(int fd, int code, void *data);

    QString decryptErrorMessage(int errNum);

signals:

    void didConnect();
    void didDisconnect();
    void messReceived(char* mess);

protected:
    QString name;

};

#endif // SPREADMANAGER_H
