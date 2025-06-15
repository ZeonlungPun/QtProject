#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "childwindow.h"
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>
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
    void on_AddBtn_clicked();


    void on_SigninBtn_clicked();



    void on_DeleteBtn_clicked();

private:
    void ShowTable();

    Ui::MainWindow *ui;
    ChildWindow *childWindow=nullptr;  // 新增一個成員變數指向子視窗
    std::string data_path;
    std::string FaceDet_path;
    std::string FaceNet_path;
    cv::Ptr<cv::FaceRecognizerSF> faceRecognizer;
    cv::Ptr<cv::FaceDetectorYN> detector;
    std::string match_img_path;
    QLabel* Imglabel;
    QSqlDatabase db;

};
#endif // MAINWINDOW_H
