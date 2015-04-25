#include "window.h"
#include "ui_window.h"
#include "sp.h"
#include "notificationmanager.h"

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
    stopDrawing(true);
}


void Window::stopDrawing(bool mine){
    if(mine)
        p_arr.append(line);
    else
        others_lines.append(line);

    line = new Line();
}

void Window::setup(){

    QVector<Line*> lines;
    others_lines = lines;
    ui->canvas->installEventFilter(this);

    QObject::connect(ui->canvas,SIGNAL(c_mousePressed(QPoint)),this,SLOT(c_mousePressed(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseMoved(QPoint)),this,SLOT(c_mouseMoved(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseRelease(QPoint)),this,SLOT(c_mouseRelease(QPoint)),Qt::QueuedConnection);


    sp = new SpreadManager();
    sp->connected = false;

    qRegisterMetaType<QVector<Line*> >("QVector<Line*>");
    QObject::connect(sp, SIGNAL(commReceived(int,QPoint,QVector<Line*>)), this, SLOT(handleComm(int,QPoint,QVector<Line*>)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didDisconnect()),this,SLOT(didDisconnect()),Qt::QueuedConnection);

    connect(sp,&SpreadManager::userJoined,[this](std::function< void(QVector<Line*>) >& lambda){
        qDebug("joineddddddddddd");
        lambda(p_arr + others_lines);
    });
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

        if (!others_lines.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::red);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            foreach (Line *l, others_lines) {
                for(int idx = 0; idx < l->points.count()-1; idx++){
                    QLine qline(l->points[idx], l->points[idx+1]);
                    painter.drawLine(qline);
                }
            }
            painter.end();
        }


        return true;
        }
    return false;
}

void Window::handleComm(int comm, QPoint p, QVector<Line*> lines){

    switch (comm) {

    case 0:
        startDrawing(p);
        break;

    case 1:
        continueDrawing(p);
        break;

    case 2:
        stopDrawing(false);
        break;

    case 3:
        others_lines = lines;
        update();
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

void Window::on_startDaemonBut_clicked()
{


    QDir cur_dir = QDir::current();
    cur_dir.cdUp();
    cur_dir.cdUp();
    cur_dir.cdUp();
    QString sp_dir = cur_dir.path() + QString("/spread");
    QString file = sp_dir + QString("/spread");
    QString config = sp_dir + QString("/spread.conf");

    QStringList arguments;
    arguments << "-c" << config;

   QProcess *process = new QProcess(this);

   bool success = process->startDetached(file, arguments);
   if(!success){

       NotificationManager *notm = &Singleton<NotificationManager>::Instance();
       std::function<void (int)> fp = [](int a) { Q_UNUSED(a); };
       notm->showAlert("Failed to start spread daemon",fp);
   }

   QString s = process->errorString();


}
