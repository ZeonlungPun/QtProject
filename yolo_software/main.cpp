#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Register cv::Mat as a Qt meta-type
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
    qRegisterMetaType<Ort::Session*>("Ort::Session&");
    MainWindow w;
    w.setWindowTitle("YOLO目標檢測系統");
    w.show();
    return a.exec();
}
