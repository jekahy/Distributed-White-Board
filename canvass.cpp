#include "canvass.h"
//#include <QtGui>

Canvass::Canvass(QWidget *parent) : QWidget(parent)
{

}

Canvass::~Canvass()
{

}


void Canvass::mousePressEvent(QMouseEvent *e){
    emit c_mousePressed(e->pos());
}

void Canvass::mouseMoveEvent(QMouseEvent *e){
    emit c_mouseMoved(e->pos());
}


void Canvass::mouseReleaseEvent(QMouseEvent *e){
    emit c_mouseRelease(e->pos());
}
