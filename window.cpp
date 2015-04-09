#include "window.h"
#include "ui_window.h"
#include "sp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include <QtConcurrent/QtConcurrent>

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    numOfLines = 0;

    this->setup();
}

Window::~Window()
{
    delete ui;
}


void Window::paintEvent(QPaintEvent *e){

    Q_UNUSED(e)
    setAttribute(Qt::WA_OpaquePaintEvent);

    QPainter painter(this);
    QPen pointPen(Qt::blue);
    pointPen.setWidth(3);

    pointPen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pointPen);

    if (!points.isEmpty()){

        if (points.count() > 1){
            QLine line(points[points.count()-2], points[points.count()-1]);
            painter.drawLine(line);

        }else{
            painter.drawPoint(points.last());
        }
    }
}


void Window::mousePressEvent(QMouseEvent *e){

    mousePressed = true;
    points.append(e->pos());
    update();
}

void Window::mouseReleaseEvent(QMouseEvent *e){
    Q_UNUSED(e)
    mousePressed = false;
    p_arr.append(points);
    points.clear();
}

void Window::mouseMoveEvent(QMouseEvent *e){

    if (mousePressed){
        points.append(e->pos());
        update();
    }
}

void Window::setup(){

    sp = new SpreadManager();

    QObject::connect(sp,SIGNAL(messReceived(char*)),this,SLOT(handleMess(char*)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);

}


void Window:: handleMess(char* mess){
 qDebug("message handled: %s",mess);
}


void Window::didConnect(){
qDebug("aloha");
}


void Window::on_button_clicked()
{
    sp->Bye();
}

void Window::on_pushButton_clicked()
{
    sp->sendMes(ui->messField->text());
}

void Window::on_connect_but_clicked()
{
    QString name = ui->nameField->text();
    QString port = ui->portField->text();
    sp->initConnection(port,name,"me");
}
