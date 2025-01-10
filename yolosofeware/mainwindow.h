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
#include <opencv2/opencv.hpp>
#include <string>
#include <onnxruntime_cxx_api.h>
#include <fstream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QPushButton* btnReadImg;
    QPushButton* btnBegin;
    QPushButton* btnModelPath;
    QPushButton* btnReadLabels;

    QLabel* Imglabel;

    QComboBox *comboBox;

    QLineEdit* ConfEdit;
    QLineEdit* NMSEdit;

    QWidget *loggingWidget;

    QScrollArea* DisplayWindow;
    QTextEdit* LoggingWindow;

    QStatusBar* statusBar;
    QProgressBar *progressBar;
    cv::Mat input_image;
    cv::Mat result_image;
    std::string onnx_path_name;
    int model_input_w, model_input_h, model_output_h, model_output_w;
    float ratio;
    std::vector<std::string> preprocess_method;
    std::vector<std::string> input_node_names;
    std::vector<std::string> output_node_names;
    Ort::Env env;
    Ort::SessionOptions session_options;
    Ort::Session session;
    int top,left;
    std::vector<std::string> labels;
    std::string selectedFramework;


    QTimer videoTimer;  // 定時器，用來控制影片播放
    cv::VideoCapture videoCapture;  // 用於讀取影片幀的OpenCV VideoCapture對象
    bool VideoFlag;
    std::string selectedVideoPath;
    void updateVideoDetiectionFrame(float ConfValue,float NMSValue,int totalFrames);
    void displayImage(const cv::Mat& frame);
    float current_frame_num;

    void Detect();
    cv::Mat preprocess(cv::Mat frame);
    void draw_detections(cv::Mat& frame,std::vector<int> indexes, std::vector<cv::Rect> boxes, std::vector<int> classIds, std::vector<float> confidences);
    std::vector<int> main_detect_process(float ConfValue, float NMSValue,cv::Mat frame,std::vector<cv::Rect>& boxes,std::vector<int>& classIds,std::vector<float>& confidences);

    QVBoxLayout *layout_left;
    QVBoxLayout *middleLayout;
    QHBoxLayout *mainlayout;
    QHBoxLayout *combolayout;
    QWidget *centralWidget;
    QFormLayout *formlayout;


public slots:
    void button_ReadImage();
    void button_LoadModel();
    void updateVideoFrame();
    void ReadLabelsfromTxt();


private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
