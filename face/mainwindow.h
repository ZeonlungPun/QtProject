#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QMainWindow>
#include "childwindow.h"
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/cvstd_wrapper.hpp>
#include <filesystem>
#include <QPixmap>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QDateTime>

namespace fs = std::filesystem;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void querrydatabase(std::string matchpath);

    void createnewtable();

private slots:
    void on_pushButton_clicked();

    void on_sigin_in_btn_clicked();

    void on_path_btn_clicked();

private:
    Ui::MainWindow *ui;
    childWindow *childwindow_ui=nullptr;
    std::string data_path;
    std::string FaceDet_path;
    std::string FaceNet_path;
    cv::Ptr<cv::FaceRecognizerSF> faceRecognizer;
    cv::Ptr<cv::FaceDetectorYN> detector;
    std::string match_img_path;
    QLabel* Imglabel;
    QSqlDatabase db2;

};
#endif // MAINWINDOW_H
