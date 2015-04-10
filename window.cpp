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

    QPoint p = e->pos();
    startDrawing(p);
    sp->startDrawing(p);

}


void Window::startDrawing(QPoint p){
    mousePressed = true;
    points.append(p);
    update();
}

void Window::mouseMoveEvent(QMouseEvent *e){

    if (mousePressed){
        QPoint p = e->pos();
        sp->continueDrawing(p);
        continueDrawing(p);
    }
}


void Window::continueDrawing(QPoint p){
    points.append(p);
    update();
}


void Window::mouseReleaseEvent(QMouseEvent *e){

    Q_UNUSED(e)
    mousePressed = false;
    QPoint p = points.last();
    sp->stopDrawing(p);
    stopDrawing();
}


void Window::stopDrawing(){
    p_arr.append(points);
    points.clear();
}

void Window::setup(){

    sp = new SpreadManager();
    sp->connected = false;

    QObject::connect(sp,SIGNAL(messReceived(char*)),this,SLOT(handleMess(char*)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didDisconnect()),this,SLOT(didDisconnect()),Qt::QueuedConnection);
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
