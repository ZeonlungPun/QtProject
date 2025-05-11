#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <iostream>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QImage>
#include <QProgressBar>
#include <string>
#include <fstream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <QMetaType>
#include <opencv2/core/core.hpp>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    cv::Mat input_image;


    std::string onnx_path_name;
    int model_input_w, model_input_h, model_output_h, model_output_w;
    float ratio;
    std::vector<std::string> input_node_names;
    std::vector<std::string> output_node_names;
    Ort::Env env;
    Ort::SessionOptions session_options;
    Ort::Session session;

signals:
    //給子線程preprocess發射訊號
    void starting(cv::Mat frame,int model_input_h, int model_input_w,int model_output_h,int model_output_w,std::vector<std::string> input_node_names,std::vector<std::string> output_node_names,Ort::Session& session);

private slots:
    void on_loadimg_btn_clicked();

    void on_loadmodel_btn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
