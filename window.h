#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QPushButton>
#include "spreadmanager.h"
#include "line.h"

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

private:
    Ui::Window *ui;
    QPoint pos;
    QPushButton *button;

    QVector<Line*> p_arr;
    QVector<Line*> others_lines;
    Line *line;

    SpreadManager *sp;

    bool mousePressed;
    int numOfLines;
    void setup();

    void startDrawing(QPoint p);
    void continueDrawing(QPoint p);
    void stopDrawing();
    void readLinesFromJson(QJsonObject json);

private slots:
    void on_button_clicked();
    void on_pushButton_clicked();
    void handleComm(int comm, QPoint p, QVector<Line*> lines);

    void on_connect_but_clicked();
    void didConnect();
    void didDisconnect();

protected:
    bool eventFilter(QObject* watched, QEvent* event);

public slots:
    void c_mousePressed(QPoint p);
    void c_mouseMoved(QPoint p);
    void c_mouseRelease(QPoint p);
//    void showAlert(QString mess, std::function< void(int) >& lambda);

};

#endif // WINDOW_H
