#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "childwindow.h"
#include <QDateTime>
#include <QDebug>
#include <QInputDialog>
#include <QThread>
#include <QMetaType>
#include <QTimer>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Employee {
    int id;
    int age;
    QString name;
    QString position;
    QString gender;
    QByteArray imgData;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createnewtable();
    void compareWithDatabase(cv::Mat& source_feature);

signals:
    void startDetect(cv::Mat raw_img);





private slots:

    void on_AddBtn_clicked();

    void on_DeleteBtn_clicked();

    void on_SigninBtn_clicked();
    void onDetectFinished(const cv::Mat& coordinates,bool faceDetected);

private:
    void ShowTable();
    void updateCurrentDateTime();

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
    cv::Mat source_img,source_feature;
    QString currentDateTime;
    QTimer* timer;

};
#endif // MAINWINDOW_H
