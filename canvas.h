#include "QFrame"
#include "QtGui"

#ifndef CANVAS_H
#define CANVAS_H


class Canvas : public QFrame
{
Q_OBJECT

public:
    explicit Canvas( QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~Canvas();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

signals:
    void c_mousePressed(QPoint p);
    void c_mouseMoved(QPoint p);
    void c_mouseRelease(QPoint p);
};

#endif // CANVAS_H
