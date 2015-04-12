#ifndef CANVASS_H
#define CANVASS_H

#include <QWidget>
#include <QtGui>
#include <QtCore>

class Canvass : public QWidget
{
    Q_OBJECT
public:
    explicit Canvass(QWidget *parent = 0);
    ~Canvass();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void c_mousePressed(QPoint p);
    void c_mouseMoved(QPoint p);
    void c_mouseRelease(QPoint p);
};

#endif // CANVASS_H
