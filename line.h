#ifndef LINE_H
#define LINE_H

#include <QObject>
#include <QtGui>

class Line : public QObject
{
    Q_OBJECT



public:

    Line(QObject* parent = 0);
    ~Line();

    QVector<QPoint> points;

    QString owner;


};

#endif // LINE_H
