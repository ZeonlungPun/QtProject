#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<cv::Mat>("cv::Mat");
    MainWindow w;
    w.setWindowTitle("人臉識別簽到系統");
    w.show();
    return a.exec();
}
