#include "window.h"
#include "ui_window.h"
#include "sp.h"


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


void Window::c_mousePressed(QPoint p){

    if(!sp->connected)
        return;
    startDrawing(p);
    sp->startDrawing(p);
}


void Window::startDrawing(QPoint p){
    mousePressed = true;
    line = new Line();
    line->points.append(p);
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

    line->points.append(p);
    ui->canvas->update();
}


void Window::c_mouseRelease(QPoint p){

    if(!sp->connected)
        return;
    mousePressed = false;
    sp->stopDrawing(p);
    stopDrawing();
}


void Window::stopDrawing(){
    p_arr.append(line);
    line = new Line();
}

void Window::setup(){

    ui->canvas->installEventFilter(this);

    QObject::connect(ui->canvas,SIGNAL(c_mousePressed(QPoint)),this,SLOT(c_mousePressed(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseMoved(QPoint)),this,SLOT(c_mouseMoved(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseRelease(QPoint)),this,SLOT(c_mouseRelease(QPoint)),Qt::QueuedConnection);


    sp = new SpreadManager();
    sp->connected = false;


    QObject::connect(sp,SIGNAL(messReceived(QString)),this,SLOT(handleMess(QString)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didDisconnect()),this,SLOT(didDisconnect()),Qt::QueuedConnection);
}

bool Window::eventFilter(QObject* watched, QEvent* event){

    if (watched == ui->canvas && event->type() == QEvent::Paint) {

        setAttribute(Qt::WA_OpaquePaintEvent);

        if (line && !line->points.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::blue);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            if (line->points.count() > 1){
                QLine qline(line->points[line->points.count()-2], line->points[line->points.count()-1]);
                painter.drawLine(qline);

            }else{
                painter.drawPoint(line->points.last());
            }
            painter.end();
        }

        return true;
        }
    return false;
}

void Window:: handleMess(QString mess){

    QJsonDocument jsonResponse = QJsonDocument::fromJson(mess.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    int comm = jsonObject["com"].toInt();
    int x = jsonObject["x"].toInt();
    int y = jsonObject["y"].toInt();

    QPoint p = QPoint(x,y);

    switch (comm) {

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
