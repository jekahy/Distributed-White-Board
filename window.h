#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QPushButton>



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
    QVector<QPoint> points;
    QVector<QVector<QPoint> > p_arr;
    bool mousePressed;
    int numOfLines;
    void setup();

private slots:
    void on_button_clicked();

    void on_pushButton_clicked();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
};

#endif // WINDOW_H
