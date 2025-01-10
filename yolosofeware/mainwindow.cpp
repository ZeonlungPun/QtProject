#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,env(ORT_LOGGING_LEVEL_ERROR, "yolov-onnx")
    , session_options()
    ,session(nullptr)
    ,Imglabel(nullptr)
    ,VideoFlag(false)

{

    ui->setupUi(this);
    // 主窗口中的中心 Widget
    centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    layout_left =new QVBoxLayout();
    mainlayout =new QHBoxLayout(centralWidget);
    middleLayout=new  QVBoxLayout();
    formlayout =new QFormLayout();
    combolayout=new QHBoxLayout();

    //添加icon圖標
    QIcon icon("favicon.ico");
    this->setWindowIcon(icon);

    //添加下拉選項，選擇深度學習推斷框架
    comboBox =new QComboBox();
    QStringList strList;
    strList<<"onnxruntime"<<"opencv";
    comboBox->addItems(strList);
    QLabel *select_label=new QLabel;
    select_label->setText("請選擇深度學習推斷框架");
    combolayout->addStretch();
    combolayout->addWidget(select_label);
    combolayout->addSpacing(15);
    combolayout->addWidget(comboBox);
    combolayout->addStretch();
    selectedFramework=comboBox->currentText().toStdString();


   //按鍵訊息
    btnReadImg=new QPushButton;
    btnReadImg->setText("讀取圖像或者影片");
    btnReadImg->setMinimumSize(150, 80);


    btnBegin=new QPushButton;
    btnBegin->setText("開始檢測");
    btnBegin->setMinimumSize(150, 80);


    btnModelPath=new QPushButton;
    btnModelPath->setText("讀取模型");
    btnModelPath->setMinimumSize(150, 80);


    //狀態欄目
    statusBar = new QStatusBar;
    this->setStatusBar(statusBar);
    // 創建歡迎文本
    QLabel *welcomeLabel = new QLabel("  歡迎使用 ");
    // 設定字體大小
    QFont font = welcomeLabel->font();
    font.setPointSize(15);
    welcomeLabel->setFont(font);
    // 添加到狀態欄
    statusBar->addWidget(welcomeLabel);
    current_frame_num=0;

    // 使用 QTimer 定期更新時間
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        welcomeLabel->setText("  歡迎使用 " + currentDateTime);
    });
    timer->start(1000);

    progressBar = new QProgressBar;

    //展示窗口訊息
    NMSEdit=new QLineEdit;
    ConfEdit=new QLineEdit;


    NMSEdit->setMinimumSize(150, 80);
    NMSEdit->setClearButtonEnabled(true);
    NMSEdit->setPlaceholderText("輸入NMS閾值");


    ConfEdit->setMinimumSize(150, 80);
    ConfEdit->setClearButtonEnabled(true);
    ConfEdit->setPlaceholderText("輸入confidence閾值");

    formlayout->addRow("NMS:",NMSEdit);
    formlayout->addRow("confidence:",ConfEdit);
    formlayout->setAlignment(Qt::AlignRight);


    btnReadLabels=new QPushButton;
    btnReadLabels->setText("讀取標籤文件");
    btnReadLabels->setMinimumSize(150, 80);


    DisplayWindow=new QScrollArea();
    DisplayWindow->setMinimumSize(1280, 1000);

    // 初始化日志顯示區域
    LoggingWindow=new QTextEdit();
    LoggingWindow->setMinimumSize(1280, 500);


    QFont font2 = LoggingWindow->font();
    font2.setPointSize(20);
    LoggingWindow->setFont(font2);

    //添加按鈕和輸入框到左邊佈局
    QVBoxLayout *buttonLayout = new QVBoxLayout();

    buttonLayout->addWidget(btnModelPath);
    buttonLayout->addWidget(btnReadImg);
    buttonLayout->addWidget(btnReadLabels);
    buttonLayout->addWidget(btnBegin);

    // 左側佈局：表單佈局 + 按鍵佈局
    // 添加彈性分隔符，讓表單和按鍵區域居中顯示
    layout_left->addStretch();
    layout_left->addLayout(combolayout);
    layout_left->addSpacing(20);
    layout_left->addLayout(formlayout);
    layout_left->addSpacing(20);
    layout_left->addLayout(buttonLayout);
    layout_left->addStretch();

    //右邊佈局
    middleLayout->addWidget(DisplayWindow);
    middleLayout->addWidget(LoggingWindow);

    //主佈局
    mainlayout->addLayout(layout_left);
    mainlayout->addLayout(middleLayout);

    this->setLayout(mainlayout);

    // 連接按鈕點擊信號到槽函數
    QObject::connect(btnReadImg, &QPushButton::clicked, this, &MainWindow::button_ReadImage);
    QObject::connect(btnBegin, &QPushButton::clicked, this, &MainWindow::Detect);
    QObject::connect(btnModelPath, &QPushButton::clicked, this, &MainWindow::button_LoadModel);
    QObject::connect(btnReadLabels, &QPushButton::clicked, this, &MainWindow::ReadLabelsfromTxt);
    QObject::connect(comboBox, &QComboBox::currentTextChanged,[=](const QString &text){
        selectedFramework = text.toStdString();
        LoggingWindow->append("選擇的框架已更改為: " + text);
    });
    session_options.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

}

MainWindow::~MainWindow()
{

    delete ui;
}
void MainWindow::ReadLabelsfromTxt()
{
    QString labelPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇標籤文件"), "/home/", "Labels (*.txt )");
    std::string labelPath_str=labelPath.toStdString();
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // 打開文件
    std::ifstream labelFile(labelPath_str);

    std::string line;
    //按行讀取類別
    while (std::getline(labelFile, line))
    {
        std::cout << "Read line: " << line << std::endl;
        if (line.length())
        {
            this->labels.push_back(line);
        }
    }
    LoggingWindow->append(QString("[INFO %1]讀取成功，總共有 %2個類別").arg(currentDateTime).arg(this->labels.size()));


}


void MainWindow::updateVideoFrame()
{
    cv::Mat frame;
    if (videoCapture.read(frame)) {
        // 將OpenCV的圖片轉換為QImage
        QImage img = QImage((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888).rgbSwapped();
        Imglabel->setPixmap(QPixmap::fromImage(img));
        Imglabel->resize(DisplayWindow->size());
        Imglabel->move(0, 0);
        DisplayWindow->setWidget(Imglabel);
    } else {
        // 影片播放完畢後停止定時器
        videoTimer.stop();
        LoggingWindow->append("[INFO] 影片播放完畢");
    }
}

void MainWindow::updateVideoDetiectionFrame(float ConfValue,float NMSValue,int totalFrames)
{


    if (videoCapture.isOpened())
    {
        cv::Mat frame;
        videoCapture >> frame;

        if (frame.empty())
        {
            videoTimer.stop();  // 停止定時器，視頻播放結束
            this->current_frame_num=0;
            return;
        }

        std::vector<cv::Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        int64 start = cv::getTickCount();
        std::vector<int> indexes = this->main_detect_process(ConfValue, NMSValue, frame, boxes, classIds, confidences);
        float fps =1/((cv::getTickCount() - start) / static_cast<float>(cv::getTickFrequency()));
        float increment_per_refresh = fps * (30.0f / 1000.0f); // 每 30 毫秒增加的幀數
        this->current_frame_num+=increment_per_refresh;
        float current_advance= 100*this->current_frame_num/totalFrames;
        std::cout<<"current:"<<current_frame_num<<std::endl;
        std::cout<<"1111:"<<current_advance<<std::endl;
        this->progressBar->setValue(current_advance);


        QString fps_qstr= QString::number(fps);

        // 繪製檢測框
        this->draw_detections(frame, indexes, boxes, classIds,confidences);
        cv::putText(frame,cv::format("FPS: %.2f", fps),cv::Point2f(100,100),cv::FONT_HERSHEY_COMPLEX, 1,cv::Scalar(0, 0, 255), 2, 8);
        LoggingWindow->append(QString("[INFO ] 當前檢測速率FPS爲 %1").arg(fps_qstr).arg("圖片讀取成功！"));

        // 顯示結果到 GUI
        displayImage(frame);
    }
}


void MainWindow::displayImage(const cv::Mat& frame)
{
    // 將 OpenCV 圖像轉換為 QImage
    QImage qImage = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888).rgbSwapped();

    // 將 QImage 轉換為 QPixmap
    QPixmap pixmap = QPixmap::fromImage(qImage);

    // 顯示圖像
    if (Imglabel == nullptr) {
        Imglabel = new QLabel(DisplayWindow);
    }
    Imglabel->setPixmap(pixmap);
    Imglabel->resize(DisplayWindow->size());
    Imglabel->move(0, 0);
    DisplayWindow->setWidget(Imglabel);
}


void MainWindow::button_ReadImage()
{

    QString selectedPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇圖像或者影片"), "/home/", "Images (*.png *.jpg *.jpeg *.mp4 *.avi)");
    if (!selectedPath.isEmpty()) {
        std::string selectedPath_str = selectedPath.toStdString();
        std::cout << "Selected file path: " << selectedPath_str << std::endl;
        size_t pos = selectedPath_str.find('.');
        std::string suffix=selectedPath_str.substr(pos + 1);


        // 如果圖片已經顯示過，先刪除之前的圖片
        if (Imglabel != nullptr) {
            delete Imglabel;
        }

        Imglabel=new QLabel;
        Imglabel->setParent(DisplayWindow);
        QPixmap pixmap;
        QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        //讀取圖像
        if(suffix.compare("jpg")==0 |suffix.compare("png")==0|suffix.compare("jpeg")==0 )
        {
            this->VideoFlag=false;
            std::cout << "Succeed to load image."<<  std::endl;
            this->input_image= cv::imread(selectedPath_str);
            if(this->input_image.empty())
            {
                QMessageBox::critical(this, "Error Message", "無法打開圖像");
                return;
            }

            pixmap.load(selectedPath);
            Imglabel->setPixmap(pixmap);
            Imglabel->resize(DisplayWindow->size());
            Imglabel->move(0, 0);
            // 設置顯示圖片的窗口為 DisplayWindow
            DisplayWindow->setWidget(Imglabel);
            // 在 LoggingWindow 中追加提示文字
            LoggingWindow->append(QString("[INFO %1] %2").arg(currentDateTime).arg("圖片讀取成功！"));
        }
        else
        { //讀取影片
            this->VideoFlag=true;
            cv::VideoCapture cap;
            cap.open(selectedPath_str);
            this->selectedVideoPath=selectedPath_str;
            if (!cap.isOpened()) {
                std::cout << "Failed to open video." << std::endl;
                QMessageBox::critical(this, "Error Message", "無法打開影片");
                return;}
            // 使用 QTimer 來控制幀率
            int delay_ms = 10;  // 每幀延遲時間，調整此值來控制播放速度
            connect(&videoTimer, &QTimer::timeout, this, &MainWindow::updateVideoFrame);
            videoTimer.start(delay_ms);
            // 保存 VideoCapture 對象，這樣可以在 updateVideoFrame 中使用
            videoCapture = cap;
            LoggingWindow->append(QString("[INFO %1] %2").arg(currentDateTime).arg("影片讀取成功！"));
        }

    } else {
        std::cout << "No file selected." << std::endl;
    }
}

void MainWindow::button_LoadModel()
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


    if (this->selectedFramework=="onnxruntime")
    {
        //innitialize the onnx model
        session = Ort::Session(env, onnx_path_name.c_str(), session_options);
        Ort::AllocatorWithDefaultOptions allocator;
        LoggingWindow->append(QString("[INFO %1] 模型讀取成功！  框架係%2").arg(currentDateTime).arg(QString::fromStdString (this->selectedFramework)));

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
            LoggingWindow->append(QString("[INFO %1] Input format: BatchxCxHxW = %2 x %3 x %4 x %5").arg(currentDateTime).arg(input_dims[0]).arg(input_dims[1]).arg(input_dims[2]).arg(input_dims[3]));
        }

        // Get output information
        size_t numOutputNodes = session.GetOutputCount();
        Ort::TypeInfo output_type_info = session.GetOutputTypeInfo(0);
        auto output_tensor_info = output_type_info.GetTensorTypeAndShapeInfo();
        auto output_dims = output_tensor_info.GetShape();
        this->model_output_h = output_dims[1];
        this->model_output_w = output_dims[2];
        std::cout << "Output format : HxW = " << output_dims[1] << "x" << output_dims[2] << std::endl;
        LoggingWindow->append(QString("[INFO %1] Output format: BatchxCxHxW = %2 x %3 ").arg(currentDateTime).arg(output_dims[1]).arg(output_dims[2]));
        for (size_t i = 0; i < numOutputNodes; i++) {
            auto out_name = session.GetOutputNameAllocated(i, allocator);
            output_node_names.push_back(out_name.get());
        }

    }





}

cv::Mat MainWindow::preprocess(cv::Mat frame)
{

    //lettter_box method
    this->result_image=frame.clone();

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    float ratio = std::min(static_cast<float>(this->model_input_h) / frame.rows,
                           static_cast<float>(this->model_input_w) / frame.cols);
    int newh=(int) std::round(frame.rows*ratio);
    int neww=(int) std::round(frame.cols*ratio);
    cv::Size new_unpad(neww,newh);
    //get the padding length in each size
    float dw=(this->model_input_w-neww)/2;
    float dh=(this->model_input_h-newh)/2;

    if (neww !=this->model_input_w || newh !=this->model_input_h)
    {  //resize the image with same ratio for wdith and height
        cv::resize(frame,frame,new_unpad,cv::INTER_LINEAR);
    }
    // calculate the padding pixel around
    int top,bottom,left,right;
    top =(int) std::round(dh-0.1);
    bottom= (int) std::round(dh+0.1);
    left = (int) std::round(dw-0.1);
    right= (int) std::round(dw+0.1);

    this->top=top;
    this->left=left;
    this->ratio=ratio;

    cv::copyMakeBorder(frame, frame, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));
    return frame;
}

std::vector<int> MainWindow::main_detect_process(float ConfValue, float NMSValue,cv::Mat frame,std::vector<cv::Rect>& boxes,std::vector<int>& classIds,std::vector<float>& confidences)
{
    //vector to save the results index after NMS
    std::vector<int> indexes;
    // Preprocess the image
    cv::Mat frame_=this->preprocess(frame);
    //normalize the image to [0,1] and resize the image to the model input
    cv::Mat blob;
    cv::dnn::blobFromImage(frame_,blob, 1.0 / 255.0, cv::Size(this->model_input_w,this->model_input_h), cv::Scalar(), true, false);
    // matrix to save the innitial output from yolov8 model
    cv::Mat det_output;
    if (selectedFramework=="onnxruntime")
    {

        // Get input tensor shape info
        size_t tpixels = this->model_input_h * this->model_input_w * 3;
        std::array<int64_t, 4> input_shape_info{1, 3, this->model_input_h, this->model_input_w};
        // Create input tensor
        auto allocator_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(allocator_info, blob.ptr<float>(), tpixels, input_shape_info.data(), input_shape_info.size());

        const std::array<const char*, 1> inputNames = {this->input_node_names[0].c_str()};
        const std::array<const char*, 1> outNames = {this->output_node_names[0].c_str()};

        // Perform inference
        std::vector<Ort::Value> ort_outputs;
        try {
            ort_outputs = session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &input_tensor, 1, outNames.data(), outNames.size());
        } catch (const std::exception& e) {
            std::cerr << "Error during ONNX inference: " << e.what() << std::endl;
            return indexes;
        }

        // Get model output
        const float* pdata = ort_outputs[0].GetTensorMutableData<float>();
        cv::Mat dout(this->model_output_h, this->model_output_w, CV_32F, (float*)pdata);
        // (num_anchors, 4+num_classes)=(8400,4+num_classes)
        det_output = dout.t();

    }




    // Post-process detections
    for (int i = 0; i < det_output.rows; i++) {
        cv::Mat classes_scores = det_output.row(i).colRange(4, 4 +this->labels.size());

        cv::Point classIdPoint;
        double score;
        minMaxLoc(classes_scores, 0, &score, 0, &classIdPoint);

        if (score > 0.05) {
            float cx = det_output.at<float>(i, 0);
            float cy = det_output.at<float>(i, 1);
            float ow = det_output.at<float>(i, 2);
            float oh = det_output.at<float>(i, 3);

            // Adjust coordinates based on padding and scaling
            cx = (cx - this->left) / this->ratio;
            cy = (cy - this->top) / this->ratio;
            ow = ow / this->ratio;
            oh = oh / this->ratio;



            int x = static_cast<int>(cx - 0.5 * ow);
            int y = static_cast<int>(cy - 0.5 * oh);
            int width = static_cast<int>(ow);
            int height = static_cast<int>(oh);

            cv::Rect box(x, y, width, height);
            boxes.push_back(box);
            classIds.push_back(classIdPoint.x);
            confidences.push_back(score);
        }
    }

    // Apply Non-Maximum Suppression (NMS)
    cv::dnn::NMSBoxes(boxes, confidences, ConfValue, NMSValue, indexes);
    return indexes;

}

void MainWindow::Detect()
{
    if (this->labels.size()==0)
    {
        QMessageBox::critical(this, "Error Message", "請先讀取標籤文件");
        return;
    }

    if (this->onnx_path_name.length()==0)
    {
        QMessageBox::critical(this, "Error Message", "請先讀取模型");
        return;
    }

    float NMSValue = NMSEdit->text().toFloat();
    float ConfValue= ConfEdit->text().toFloat();
    std::cout << "NMS:"<<NMSValue<< std::endl;
    std::cout <<"confidence:"<<ConfValue<< std::endl;
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if(NMSValue<=0 | NMSValue>=1)
    {
        QMessageBox::critical(this, "Error Message", "輸入的NMS值需要在0到1之間");
        return;
    }
    else
    {
        LoggingWindow->append(QString("[INFO %1] NMS閾值設定爲%2").arg(currentDateTime).arg(NMSValue));
    }

    if(ConfValue<=0 | ConfValue>=1)
    {
        QMessageBox::critical(this, "Error Message", "輸入的confidencec值需要在0到1之間");
        return;
    }
    else
    {
        LoggingWindow->append(QString("[INFO %1] confidencec閾值設定爲%2").arg(currentDateTime).arg(ConfValue));
    }

    if(this->input_image.empty() && this->selectedVideoPath.length()==0)
    {
        QMessageBox::critical(this, "Error Message", "請首先精確讀取圖像");
        return;
    }

    LoggingWindow->append(QString("[INFO %1] 開始檢測 ").arg(currentDateTime));

    if (this->VideoFlag)
    {
        LoggingWindow->append(QString("[INFO %1] 正在處理影片數據").arg(currentDateTime));
        QLabel *detectionLabel = new QLabel("  檢測進度 ");
        this->statusBar->addPermanentWidget(detectionLabel);

        this->progressBar->setRange(0, 100);
        this->progressBar->setValue(0);
        this->statusBar->insertPermanentWidget(0,progressBar);
        this->progressBar->show();

        //初始化 VideoCapture
        cv::VideoCapture cap;
        cap.open(this->selectedVideoPath);
        // 獲取影片的總幀數
        int totalFrames_ = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        int fps = cap.get(cv::CAP_PROP_FPS);
        int totalFrames= (int) 1000.0f/30.0f* (totalFrames_/fps);
        std::cout<<"total frames:"<<totalFrames<<std::endl;
        // 定時更新視頻框架
        connect(&videoTimer, &QTimer::timeout, [=]() {
                this->updateVideoDetiectionFrame(ConfValue,NMSValue,totalFrames);
            });
        videoTimer.start(30);  // 每30毫秒更新一次

       // 創建 VideoCapture 對象，用於處理每一幀
        videoCapture = cap;

    }
    else
    {
        LoggingWindow->append(QString("[INFO %1] 正在處理圖像數據").arg(currentDateTime));
        std::vector<cv::Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<int>indexes=this->main_detect_process(ConfValue,NMSValue,this->input_image,boxes,classIds,confidences);
        // Draw detections
        this->draw_detections(this->result_image,indexes, boxes, classIds,confidences);
        // visulized the detection results on the display window
        Imglabel=new QLabel;
        Imglabel->setParent(DisplayWindow);

        // Convert cv::Mat to QImage
        QImage qImage = QImage(this->result_image.data, this->result_image.cols, this->result_image.rows, this->result_image.step, QImage::Format_RGB888).rgbSwapped();

        // Convert QImage to QPixmap
        QPixmap pixmap = QPixmap::fromImage(qImage);


        Imglabel->setPixmap(pixmap);
        Imglabel->resize(DisplayWindow->size());
        Imglabel->move(0, 0);
    }



    // 設置顯示圖片的窗口為 DisplayWindow
    DisplayWindow->setWidget(Imglabel);
    LoggingWindow->append(QString("[INFO %1] 目標檢測與結果可視化完成 ").arg(currentDateTime));
}


void MainWindow::draw_detections(cv::Mat& frame,std::vector<int> indexes, std::vector<cv::Rect> boxes, std::vector<int> classIds, std::vector<float> confidences)
{

    // 打開 CSV 文件（如果文件不存在會創建它）
    std::ofstream csv_file;
    csv_file.open("detections_result.csv", std::ios::out | std::ios::app);
    std::string currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    // 如果文件成功打開，寫入標題（如果是第一次創建文件）
    if (csv_file.is_open()) {
        // 檢查文件是否為空（即是否是第一次寫入）
        if (csv_file.tellp() == 0) {
            csv_file << "Time,Label,Confidence,Left,Top,Width,Height\n";  // CSV 頭部
        }

        for (size_t i = 0; i < indexes.size(); i++) {
            int index = indexes[i];
            int label_idx = classIds[index];
            float score=confidences[index];
            std::string label_name = this->labels[label_idx];

            // 繪製邊界框
            cv::rectangle(frame, boxes[index], cv::Scalar(0, 0, 255), 2, 8);
            cv::putText(frame,
                        label_name, // 標籤名稱
                        cv::Point(boxes[index].x, boxes[index].y + 5), // 顯示位置
                        cv::FONT_HERSHEY_COMPLEX, // 使用複雜字體
                        1, // 字體大小（可以根據需要調整）
                        cv::Scalar(0, 0, 255), // 顏色（紅色）
                        2, // 字體粗細
                        8); // 線型（8表示開放型字形）

            // 保存檢測結果到 CSV 文件
            csv_file << currentDateTime << ","        // 顯示檢測時間
                     << label_name << ","    // 顯示標籤
                     << score <<","
                     << boxes[index].x << ","// 顯示左上角 X 坐標
                     << boxes[index].y << ","// 顯示左上角 Y 坐標
                     << boxes[index].width << ","  // 顯示寬度
                     << boxes[index].height << "\n";  // 顯示高度
        }
        // 關閉 CSV 文件
        csv_file.close();
    } else {
        std::cerr << "Unable to open CSV file!" << std::endl;
    }
}




