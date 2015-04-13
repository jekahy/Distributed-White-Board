#include "canvas.h"
#include "qwidget.h"

Canvas::Canvas(QWidget *parent, Qt::WindowFlags f) : QFrame(parent)
{
    Q_UNUSED(f);

}

Canvas::~Canvas()
{

}


void Canvas::mousePressEvent(QMouseEvent *e){
    emit c_mousePressed(e->pos());
}

void Canvas::mouseMoveEvent(QMouseEvent *e){
    emit c_mouseMoved(e->pos());
}


void Canvas::mouseReleaseEvent(QMouseEvent *e){
    emit c_mouseRelease(e->pos());
}
