#include "window.h"
#include "ui_window.h"
#include "sp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include "canvass.h"

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
//    setAttribute(Qt::WA_OpaquePaintEvent);

//    QPainter painter(this);
//    QPen pointPen(Qt::blue);
//    pointPen.setWidth(3);

//    pointPen.setJoinStyle(Qt::RoundJoin);
//    painter.setPen(pointPen);

//    if (!points.isEmpty()){

//        if (points.count() > 1){
//            QLine line(points[points.count()-2], points[points.count()-1]);
//            painter.drawLine(line);

//        }else{
//            painter.drawPoint(points.last());
//        }
//    }
}


void Window::c_mousePressed(QPoint p){

    if(!sp->connected)
        return;
    startDrawing(p);
    sp->startDrawing(p);
}


void Window::startDrawing(QPoint p){
    mousePressed = true;
    points.append(p);
    ui->canvas->update();
}

void Window::c_mouseMoved(QPoint p){

    if(!sp->connected)
        return;
    if (mousePressed){
        sp->continueDrawing(p);
        continueDrawing(p);
    }
}


void Window::continueDrawing(QPoint p){

    points.append(p);
    ui->canvas->update();
}


void Window::c_mouseRelease(QPoint p){

    if(!sp->connected)
        return;
    mousePressed = false;
//    QPoint p = points.last();
    sp->stopDrawing(p);
    stopDrawing();
}


void Window::stopDrawing(){
    p_arr.append(points);
    points.clear();
}

void Window::setup(){

    ui->canvas->installEventFilter(this);

    QObject::connect(ui->canvas,SIGNAL(c_mousePressed(QPoint)),this,SLOT(c_mousePressed(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseMoved(QPoint)),this,SLOT(c_mouseMoved(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseRelease(QPoint)),this,SLOT(c_mouseRelease(QPoint)),Qt::QueuedConnection);


    sp = new SpreadManager();
    sp->connected = false;

    QObject::connect(sp,SIGNAL(messReceived(char*)),this,SLOT(handleMess(char*)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didDisconnect()),this,SLOT(didDisconnect()),Qt::QueuedConnection);
}

bool Window::eventFilter(QObject* watched, QEvent* event){

    if (watched == ui->canvas && event->type() == QEvent::Paint) {

        setAttribute(Qt::WA_OpaquePaintEvent);

        if (!points.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::blue);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            if (points.count() > 1){
                QLine line(points[points.count()-2], points[points.count()-1]);
                painter.drawLine(line);

            }else{
                painter.drawPoint(points.last());
            }
            painter.end();
        }

            return true; // return true if you do not want to have the child widget paint on its own afterwards, otherwise, return false.
        }
        return false;
}

void Window:: handleMess(char* mess){

    qDebug("message handled: %s",mess);

    std::string str(mess);
    std::string type = str.substr(str.find_first_of('com')+1, 1 );

    printf("\"%s\"\n", type.c_str());

    std::string xs("x=");
    std::string ys("y=");

    std::string::size_type start = str.find(xs)+2;
    std::string::size_type length = str.find(ys)-1 - start;
    std::string x_str = str.substr(start, length);

    start = str.find(ys)+2;
    length = str.length() - start;

    std::string y_str = str.substr(start, length);

    int x = std::stoi( x_str );
    int y = std::stoi( y_str );
    int t = std::stoi( type );

    QPoint p = QPoint(x,y);

    switch (t) {

    case 0:
        startDrawing(p);
        break;

    case 1:
        continueDrawing(p);
        break;

    case 2:
        stopDrawing();
        break;

    default:
        break;
    }
}


void Window::didConnect(){

    ui->nameField->setDisabled(sp->connected);
    ui->portField->setDisabled(sp->connected);
    ui->messField->setDisabled(!sp->connected);
    ui->pushButton->setDisabled(!sp->connected);
    ui->connect_but->setText("Disconnect");
}


void Window::didDisconnect(){
    ui->nameField->setDisabled(sp->connected);
    ui->portField->setDisabled(sp->connected);
    ui->messField->setDisabled(!sp->connected);
    ui->pushButton->setDisabled(!sp->connected);
    ui->connect_but->setText("Connect");
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
    if (!sp->connected){
        QString name = ui->nameField->text();
        QString port = ui->portField->text();
        sp->initConnection(port,name,"me");
    }else{
        sp->closeConnection();
    }

}
