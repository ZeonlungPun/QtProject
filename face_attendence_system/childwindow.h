#ifndef CHILDWINDOW_H
#define CHILDWINDOW_H
#include <QBuffer>
#include <QDateTime>
#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <iostream>
#include <QtSql/QSqlQueryModel>
#include <QSqlQuery>
#include <QFileDialog>
#include <QSqlTableModel>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/cvstd_wrapper.hpp>
namespace Ui {
class ChildWindow;
}

class ChildWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChildWindow(QMainWindow *parent = nullptr);
    ~ChildWindow();


private slots:
   

    void onTableRowClicked(const QModelIndex &index);
    void on_insertBtn_clicked();

    void on_deleteBtn_clicked();

    void on_modifyBtn_clicked();



private:
    Ui::ChildWindow *ui;
    QSqlDatabase db;
    cv::Ptr<cv::FaceDetectorYN> detector;

    //創建數據庫
    void createDB();
  //創建表
    void createTable();
   //查詢
    void queryTable();
    void showTable();

};

#endif // CHILDWINDOW_H
