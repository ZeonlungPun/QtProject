#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


    w.setWindowTitle("You Only Look Once");
    w.resize(1800, 1800);


    w.show();
    return a.exec();
}
