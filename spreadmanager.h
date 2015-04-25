#ifndef SPREADMANAGER_H
#define SPREADMANAGER_H

#include <QObject>
#include <QtCore>
#include "sp.h"
#include <atomic>
#include <QtGui>
#include "line.h"

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
    void sendJSON(QJsonObject json);
    void sendPreviousLines(QVector<Line*> lines, QString target);

    int myGroupNum = 0;

private:

    QMutex q_mutex;
    char *group_name;
    mailbox Mbox;
    char Private_group[MAX_GROUP_NAME];
    int     To_exit = 0;

    char* toChar(QString str);
    void Read_thread_routine();
    void Read_message(int fd, int code, void *data);

    void handleMessage(QString mess);

    QVector<Line*> readLinesFromJson(QJsonObject json);

    QString decryptErrorMessage(int errNum);
    QJsonObject convertComToJSON(int comm, QPoint p);
    QJsonObject convertLinesToJSON(QVector<Line*> lines);
    QString getNameFromStr(QString str);

    void handleSecMessage(QString mess);

signals:

    void didConnect();
    void didDisconnect();
//    void messReceived(QString mess);
    void commReceived(int comm, QPoint p, QVector<Line*> lines);

    void userJoined(std::function< void(QVector<Line*>) >& lambda);
    void receivedPreviousLines(QVector<Line*> lines);

protected:
    QString name;

};

#endif // SPREADMANAGER_H
