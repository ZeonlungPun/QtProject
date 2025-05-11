#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "thread.h"
#include <QDebug>
#include <QThread>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

    ,env(ORT_LOGGING_LEVEL_ERROR, "yolov-onnx")
    , session_options()
    ,session(nullptr)
{
    ui->setupUi(this);

    qDebug() << "主線程地址: " << QThread::currentThread();
    //創建子線程列表
    QThread* detect_task=new QThread;

   //創建任務類的對象
    Detect* Detect_work=new Detect;

    //任務類的對象移動到子線程
    Detect_work->moveToThread(detect_task);

    //主頁面發射訊號給子線程Preprocess_work
    connect(this,&MainWindow::starting,Detect_work,&Detect::working);
    //啓動preprocess子線程，當按下檢測按鈕
    connect(ui->detect_btn,&QPushButton::clicked,this,[=]()
    {
        emit starting(this->input_image,this->model_input_h,this->model_input_w,this->model_output_h,this->model_output_w, this->input_node_names,this->output_node_names, this->session);
        detect_task->start();
    });

    //處理完成後將子線程Postprocess_work結果發送給主線程，並顯示在主線程的窗口上
    connect(Detect_work,&Detect::curResult,this,[=](cv::Mat raw_img)
    {
        // visulized the detection results on the display window
       qDebug() << "UI thread address: " << QThread::currentThread();

       QLabel* Imglabel=new QLabel;
       Imglabel->setParent(ui->displaywindow);

       // Convert cv::Mat to QImage
       QImage qImage = QImage(raw_img.data, raw_img.cols, raw_img.rows, raw_img.step, QImage::Format_RGB888).rgbSwapped();

       // Convert QImage to QPixmap
       QPixmap pixmap = QPixmap::fromImage(qImage);


       Imglabel->setPixmap(pixmap);
       Imglabel->resize(ui->displaywindow->size());
       Imglabel->move(0, 0);
       ui->displaywindow->setWidget(Imglabel);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loadimg_btn_clicked()
{
    QString selectedPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇圖像"), "/home/", "Images (*.png *.jpg *.jpeg)");
    if (!selectedPath.isEmpty()) {

        std::string selectedPath_str = selectedPath.toStdString();
        std::cout << "Selected file path: " << selectedPath_str << std::endl;
        size_t pos = selectedPath_str.find('.');
        std::string suffix=selectedPath_str.substr(pos + 1);


        QLabel* Imglabel=new QLabel;

        Imglabel->setParent(ui->displaywindow);
        QPixmap pixmap;
        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        //讀取圖像
        if(suffix.compare("jpg")==0 |suffix.compare("png")==0|suffix.compare("jpeg")==0 )
        {

            std::cout << "Succeed to load image."<<  std::endl;
            this->input_image= cv::imread(selectedPath_str);
            if(this->input_image.empty())
            {
                QMessageBox::critical(this, "Error Message", "無法打開圖像");
                return;
            }

            pixmap.load(selectedPath);
            Imglabel->setPixmap(pixmap);
            Imglabel->resize(ui->displaywindow->size());
            Imglabel->move(0, 0);
            // 設置顯示圖片的窗口為 DisplayWindow
            ui->displaywindow->setWidget(Imglabel);
            // 在 LoggingWindow 中追加提示文字
            ui->loggingwindow->append(QString("[INFO %1] %2").arg(currentDateTime).arg("圖片讀取成功！"));
        }
    }
    else
    {
        std::cout << "No file selected." << std::endl;
    }
}



void MainWindow::on_loadmodel_btn_clicked()
{

    QString selectedPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇模型"), "/home/", "Images (*.onnx)");
    if (selectedPath.isEmpty()) {
        QMessageBox::critical(this, "Error Message", "模型讀取失敗!!!");
        return;
    }



    std::string modelPath_str =selectedPath.toStdString();
    this->onnx_path_name=modelPath_str;
    std::cout << "Succeed to load onnx model：" <<modelPath_str<<std::endl;
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //innitialize the onnx model
    session = Ort::Session(env, onnx_path_name.c_str(), session_options);
    Ort::AllocatorWithDefaultOptions allocator;
    ui->loggingwindow->append(QString("[INFO %1] 模型讀取成功！  ").arg(currentDateTime));

   // Get input information
    size_t numInputNodes = session.GetInputCount();
    for (size_t i = 0; i < numInputNodes; i++) {
           auto input_name = session.GetInputNameAllocated(i, allocator);
           input_node_names.push_back(input_name.get());
           Ort::TypeInfo input_type_info = session.GetInputTypeInfo(i);
           auto input_tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
           auto input_dims = input_tensor_info.GetShape();

           this->model_input_w = input_dims[3];
           this->model_input_h = input_dims[2];
           ui->loggingwindow->append(QString("[INFO %1] Input format: BatchxCxHxW = %2 x %3 x %4 x %5").arg(currentDateTime).arg(input_dims[0]).arg(input_dims[1]).arg(input_dims[2]).arg(input_dims[3]));
   }

   // Get output information
    size_t numOutputNodes = session.GetOutputCount();
    Ort::TypeInfo output_type_info = session.GetOutputTypeInfo(0);
    auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
    auto output_dims = output_tensor_info.GetShape();
    this->model_output_h = output_dims[1];
    this->model_output_w = output_dims[2];
    std::cout << "Output format : HxW = " << output_dims[1] << "x" << output_dims[2] << std::endl;
    ui->loggingwindow->append(QString("[INFO %1] Output format: BatchxCxHxW = %2 x %3 ").arg(currentDateTime).arg(output_dims[1]).arg(output_dims[2]));
    for (size_t i = 0; i < numOutputNodes; i++) {
        auto out_name = session.GetOutputNameAllocated(i, allocator);
        output_node_names.push_back(out_name.get());
    }
}

