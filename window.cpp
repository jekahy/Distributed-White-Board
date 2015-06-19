#include "window.h"
#include "ui_window.h"
#include "sp.h"
#include "notificationmanager.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    this->setup();
}

Window::~Window()
{
    stopDaemon();
    delete ui;
}


void Window::c_mousePressed(QPoint p){

    if(!sp->connected)
        return;
    startDrawing(p,false);
    sp->startDrawing(p);
}


void Window::startDrawing(QPoint p, bool remote){

    if(remote){
        remote_line.points.clear();
        remote_line.points.append(p);
    }else{
        mousePressed = true;
        line.points.clear();
        line.points.append(p);
    }
    ui->canvas->update();
}

void Window::c_mouseMoved(QPoint p){

    if(!sp->connected)
        return;
    if (mousePressed){
        sp->continueDrawing(p);
        continueDrawing(p, false);
    }
}


void Window::continueDrawing(QPoint p, bool remote){

    if(remote)
        remote_line.points.append(p);
    else
        line.points.append(p);

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
        my_lines.append(line);
    else
        remote_lines.append(line);

    line.points.clear();
}

void Window::setup(){


    remote_lines = QVector<Line>();
    ui->canvas->installEventFilter(this);

    QObject::connect(ui->canvas,SIGNAL(c_mousePressed(QPoint)),this,SLOT(c_mousePressed(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseMoved(QPoint)),this,SLOT(c_mouseMoved(QPoint)),Qt::QueuedConnection);
    QObject::connect(ui->canvas,SIGNAL(c_mouseRelease(QPoint)),this,SLOT(c_mouseRelease(QPoint)),Qt::QueuedConnection);


    sp = new SpreadManager();

    qRegisterMetaType<QVector<Line> >("QVector<Line>");
    QObject::connect(sp, SIGNAL(commReceived(int,QPoint,QVector<Line>)), this, SLOT(handleComm(int,QPoint,QVector<Line>)),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didConnect()),this,SLOT(didConnect()),Qt::QueuedConnection);
    QObject::connect(sp,SIGNAL(didDisconnect()),this,SLOT(didDisconnect()),Qt::QueuedConnection);

    connect(sp,&SpreadManager::userJoined,[this](std::function< void(QVector<Line>) >& lambda){
        lambda(my_lines + remote_lines);
    });

    if(checkIfDaemonIsRunning())
        ui->startDaemonBut->setText("Stop Daemon");
}

bool Window::eventFilter(QObject* watched, QEvent* event){

    if (watched == ui->canvas && event->type() == QEvent::Paint) {

        setAttribute(Qt::WA_OpaquePaintEvent);

        if (!line.points.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::blue);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            if (line.points.count() > 1){
                QLine qline(line.points[line.points.count()-2], line.points[line.points.count()-1]);
                painter.drawLine(qline);

            }else{
                painter.drawPoint(line.points.last());
            }
            painter.end();
        }

        if (!remote_line.points.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::red);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            if (remote_line.points.count() > 1){
                QLine qline(remote_line.points[remote_line.points.count()-2], remote_line.points[remote_line.points.count()-1]);
                painter.drawLine(qline);

            }else{
                painter.drawPoint(remote_line.points.last());
            }
            painter.end();
        }

        if (!remote_lines.isEmpty()){

            QPainter painter;
            painter.begin(ui->canvas);

            QPen pointPen(Qt::red);
            pointPen.setWidth(3);

            pointPen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pointPen);

            foreach (Line l, remote_lines) {
                for(int idx = 0; idx < l.points.count()-1; idx++){
                    QLine qline(l.points[idx], l.points[idx+1]);
                    painter.drawLine(qline);
                }
            }
            painter.end();
        }


        return true;
        }
    return false;
}

void Window::handleComm(int comm, QPoint p, QVector<Line> lines){

    switch (comm) {

    case 0:
        startDrawing(p, true);
        break;

    case 1:
        continueDrawing(p, true);
        break;

    case 2:
        stopDrawing(false);
        break;

    case 3:
        remote_lines = lines;
        update();
        break;

    default:
        break;
    }
}


void Window::didConnect(){

    ui->nameField->setDisabled(sp->connected);
    ui->portField->setDisabled(sp->connected);
    ui->connect_but->setText("Disconnect");
}


void Window::didDisconnect(){
    ui->nameField->setDisabled(sp->connected);
    ui->portField->setDisabled(sp->connected);
    ui->connect_but->setText("Connect");
}


void Window::on_button_clicked()
{

    stopDaemon();
    if (sp->connected)
        sp->Bye();
    else
        qApp->exit();
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
    if(checkIfDaemonIsRunning()){
        stopDaemon();
    }else{
        createConfigFile();

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

           std::function<void (QMessageBox*)> btnFunc = [](QMessageBox *mb) {
               mb->addButton("OK",QMessageBox::AcceptRole);
               mb->setText("Failed to start spread daemon");
           };
           NotificationManager::showAlert(btnFunc);
       }
       ui->startDaemonBut->setText("Stop Daemon");
    }
}


void Window::stopDaemon(){

    system("pkill spread");
    ui->startDaemonBut->setText("Start Daemon");
}


QString Window::findMyIp(){

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)){
             return address.toString();
        }
    }
    return "";
}


QString Window::findNetMask(QString ip){

    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        foreach(const QNetworkAddressEntry &aEntry, interface.addressEntries()){
            if(ip== aEntry.ip().toString()){
                return aEntry.netmask().toString();
            }
        }
    }
    return "";
}

void Window::createConfigFile(){

    QString ip = findMyIp();
    QString mask = findNetMask(ip);

    QDir cur_dir = QDir::current();
    cur_dir.cdUp();
    cur_dir.cdUp();
    cur_dir.cdUp();
    QString sp_dir = cur_dir.path() + QString("/spread");

    QString name = sp_dir + QString("/spread.conf");
    QFile file(name);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << "Spread_Segment  "<< mask << ":" << ui->portField->text() << "{\n";
    out << "  me    " << ip << "\n";


//    std::string ipp = "192.168.0.1";
//    QStringList d = ip.split(".");
    ip.truncate(ip.lastIndexOf(".")+1);

//    out << "  me    " << ip + "63#10" << "\n";

//    for(int idx=1; idx<= 128; idx++){
//        out << "\nSpread_Segment  "<< mask << ":" << ui->portField->text() << "{\n";
//        out << "  machine" << idx << "   " << ip+QString::number(idx) << "\n";
//        out << "}\n";
//    }

    out << "}\n";


//    out << "Spread_Segment2  "<< mask << ":" << ui->portField->text() << "{\n";
//    out << "  me    " << ip << "\n";


//    std::string ipp = "192.168.0.1";
//    QStringList d = ip.split(".");
//    ip.truncate(ip.lastIndexOf(".")+1);

//    out << "  me    " << ip + "63#10" << "\n";

//    for(int idx=129; idx<= 256; idx++){
//        out << "  machine" << idx << "   " << ip+QString::number(idx) << "\n";
//    }

//    out << "}";
}


bool Window::checkIfDaemonIsRunning(){

    QProcess process1;
    QProcess process2;
    QProcess process3;

    process1.setStandardOutputProcess(&process2);
    process2.setStandardOutputProcess(&process3);
    process1.start("ps", QStringList() << "aux");
    process2.start("grep", QStringList() << "spread");
    process3.start("grep", QStringList() << "-v" << "grep");
    process3.waitForFinished();

    QString output(process3.readAllStandardOutput());

    process1.close();
    process2.close();
    process3.close();

   return output != "";
}
