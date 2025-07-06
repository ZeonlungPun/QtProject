#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "thread.h"

double l2norm_similar_thresh = 1.128;
float scoreThreshold = 0.2;
float nmsThreshold = 0.5;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "the address of main process: " << QThread::currentThread();

    //create sub-process list
    QThread* detect_task=new QThread;


    //create the object of task Class
    Detect* Detect_work=new Detect;


    //move the object of task class to sub-process
    Detect_work->moveToThread(detect_task);


    FaceDet_path="/home/punzeonlung/CPP/face_detection_yunet_2023mar_int8.onnx";
    FaceNet_path="/home/punzeonlung/CPP/face_recognition_sface_2021dec.onnx";
    detector=cv::FaceDetectorYN::create(FaceDet_path, "", cv::Size(640, 640), scoreThreshold, nmsThreshold, 1);
    faceRecognizer =cv::FaceRecognizerSF::create(FaceNet_path, "");
    if (detector.empty()) {
       std::cerr << "Face detection model failed to load!" << std::endl;
       return;
    } else {
       std::cout << "Face detection model loaded successfully!" << std::endl;
    }
    if (faceRecognizer.empty()) {
       std::cerr << "Face recognition model failed to load!" << std::endl;
       return;
    } else {
       std::cout << "Face recognition model loaded successfully!" << std::endl;
    }
    // set the detector model
    Detect_work->setDetector(detector);

    //main process emit signal to sub-process and then can implement it once it recieve the signals
    connect(this, &MainWindow::startDetect, Detect_work, &Detect::working);

    //only start the sub-process when it recieve the signals
    detect_task->start();

    // post-process the detect/recognize results
    connect(Detect_work, &Detect::detectFinished, this, &MainWindow::onDetectFinished);


    Imglabel=nullptr;
    //加載員工資料表
    db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("employee_log.db");
    if (db.open()==true)
    {
       std::cout<<"employee_log open successfully"<<std::endl;
    }
    else
    {
       std::cout<<"employee_log open failurely"<<std::endl;
    }

    // 初始化時間
    updateCurrentDateTime();

    // 啓動定時器，每分鐘更新一次
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateCurrentDateTime);
    timer->start(1 * 1000);  // 每1秒更新一次



    this->createnewtable();
    this->ShowTable();
}
void MainWindow::updateCurrentDateTime()
{
    currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

// QImage -> cv::Mat
cv::Mat QImageToMat(const QImage &image) {
    switch (image.format()) {
    case QImage::Format_RGB32: {
        cv::Mat mat(image.height(), image.width(), CV_8UC4,
                    const_cast<uchar*>(image.bits()), image.bytesPerLine());
        return mat.clone(); // clone 保證資料有效
    }
    case QImage::Format_RGB888: {
        QImage swapped = image.rgbSwapped(); // Qt 是 RGB，OpenCV 是 BGR
        return cv::Mat(swapped.height(), swapped.width(), CV_8UC3,
                       const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
    }
    case QImage::Format_Grayscale8: {
        return cv::Mat(image.height(), image.width(), CV_8UC1,
                       const_cast<uchar*>(image.bits()), image.bytesPerLine()).clone();
    }
    default:
        qDebug() << "Unsupported QImage format for conversion.";
        return cv::Mat();
    }
}


void MainWindow::onDetectFinished( const cv::Mat& coordinates,bool faceDetected)
{

    if (faceDetected)
    {
        cv::Mat sourceface= this->source_img(cv::Rect(coordinates.at<float>(0, 0),coordinates.at<float>(0, 1),coordinates.at<float>(0, 2),coordinates.at<float>(0, 3)));
        //將cv::mat轉化爲QImage
        QImage sourcefaceQImage=QImage(sourceface.data,sourceface.cols,sourceface.rows,sourceface.step,QImage::Format_RGB888).rgbSwapped();
        // 將 QImage 轉換為 QPixmap
        QPixmap sourceface_pixmap = QPixmap::fromImage(sourcefaceQImage);
        ui->DisplayWindow1->setPixmap(sourceface_pixmap);
        // 自動縮放圖片以適應 QLabel 大小
        ui->DisplayWindow1->setScaledContents(true);
        ui->LoggingWindow->append(QString("[INFO %1] %2").arg(this->currentDateTime).arg("簽到人頭像讀取成功！"));

        //get the face feature through face image and face recognition model
        cv::Mat source_feature_;
        faceRecognizer->feature(sourceface, source_feature_);
        this->source_feature= source_feature_.clone();
        compareWithDatabase(this->source_feature);
    }
    else {

        ui->LoggingWindow->append("[WARNING] 未偵測到人臉！");
        std::cout<<"no faces detection"<<std::endl;
        return;
    }

}

void MainWindow::compareWithDatabase(cv::Mat& source_feature)
{
    std::vector<float>recognize_scores;
    double min_score = std::numeric_limits<double>::max();  // 初始設為最大
    Employee best_match;  // 儲存最佳匹配的那一筆資料

    //traverse through the face image stored in the base
    QSqlQuery query;
    if (!query.exec("SELECT id, age, name, position, gender, img FROM employee")) {
            qDebug() << "Query failed:" << query.lastError().text();
            return;
        }
    int count=0;
    while (query.next()) {

        QByteArray imgData = query.value(5).toByteArray();

        QImage image;
        image.loadFromData(imgData);
        cv::Mat face_mat = QImageToMat(image);
        if (face_mat.channels() == 4) {
            cv::cvtColor(face_mat, face_mat, cv::COLOR_BGRA2BGR);
        }
        cv::Mat dest_feature;
        faceRecognizer->feature(face_mat, dest_feature);
        dest_feature = dest_feature.clone();
        // 計算 L2 距離
        double L2_score = faceRecognizer->match(source_feature, dest_feature, cv::FaceRecognizerSF::DisType::FR_NORM_L2);
        recognize_scores.push_back(L2_score);
        if (L2_score < min_score) {

            min_score =L2_score;
            best_match.id = query.value(0).toInt();
            best_match.age = query.value(1).toInt();
            best_match.name = query.value(2).toString();
            best_match.position = query.value(3).toString();
            best_match.gender = query.value(4).toString();
            best_match.imgData = imgData;
            }
        std::cout<<"111:"<<count<<":"<<L2_score<<std::endl;
        count+=1;


        }

    if (min_score < std::numeric_limits<double>::max()) {

        qDebug() << "Best Match:";
        qDebug() << "ID:" << best_match.id;
        qDebug() << "Name:" << best_match.name;
        qDebug() << "Age:" << best_match.age;
        qDebug() << "Position:" << best_match.position;
        qDebug() << "Gender:" << best_match.gender;
        qDebug() << "Score:" << min_score;

        // 顯示圖片
        QImage best_img;
        QPixmap Portrait2pixmap;
        Portrait2pixmap.loadFromData(best_match.imgData, "PNG");  // 根據存的格式，如 PNG
        ui->DisplayWindow2->setPixmap(Portrait2pixmap);
        ui->DisplayWindow2->setScaledContents(true);
        ui->LoggingWindow->append(QString("[INFO %1] %2").arg(this->currentDateTime).arg("簽到人資料讀取成功！"));
        ui->LoggingWindow->append(QString("[INFO %1] %2 簽到").arg(this->currentDateTime).arg(best_match.name));
        QString sql_str= QString("INSERT INTO signin VALUES(%1,'%2','%3') ").arg(best_match.id).arg(best_match.name).arg(this->currentDateTime);
        QSqlQuery insert_query;
        if (insert_query.exec(sql_str)) {
            ui->LoggingWindow->append(QString("[INFO %1] %2 已成功簽到並寫入資料庫").arg(this->currentDateTime).arg(best_match.name));
            ShowTable();  // 👉 更新表格顯示
        } else {
            qDebug() << "寫入 signin 失敗：" << insert_query.lastError().text();
            ui->LoggingWindow->append(QString("[ERROR %1] %2 寫入資料庫失敗").arg(this->currentDateTime).arg(best_match.name));
        }

    }
}


void MainWindow::createnewtable()
{
    QSqlQuery query;
    // 檢查是否已存在 employee 表
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='signin';"))
    {
        if (query.next())
        {
            std::cout << "[INFO] Table 'signin' already exists." << std::endl;
            return;
        }
    }
    QString sql_str= QString( "CREATE TABLE signin("
                              "id INT  NOT NULL,"
                              "name TEXT NOT NULL,"
                              "SignIntime TEXT NOT NULL)");

    if (query.exec(sql_str)==true)
    {
        std::cout<<"create Table signin successfully"<<std::endl;
    }
    else
    {
        std::cout<<"create  signin failurely"<<std::endl;
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}








void MainWindow::ShowTable()
{
    // 若 db 尚未開啟，提前檢查
    if (!db.isOpen()) {
        qDebug() << "資料庫尚未開啟！";
        return;
    }

    // 建立 model
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("signin");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);  // 禁止直接編輯
    model->select();

    if (model->lastError().isValid()) {
        qDebug() << "讀取資料失敗：" << model->lastError().text();
        delete model;
        return;
    }

    // 綁定至 tableView
    ui->tableWindow->setModel(model);

    // 設定表格樣式
    ui->tableWindow->resizeColumnsToContents();
    ui->tableWindow->resizeRowsToContents();
    QHeaderView* header = ui->tableWindow->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch); // 均分欄位寬度
}


void MainWindow::on_AddBtn_clicked()
{

    if (!childWindow)
    {
        childWindow = new ChildWindow(this);  // 可傳 this 做為父視窗

    }
    childWindow->show();     // 顯示新視窗
    childWindow->raise();    // 提到最前
    childWindow->activateWindow();  // 嘗試聚焦
}

void MainWindow::on_DeleteBtn_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "刪除簽到記錄",
                                         "請輸入需要刪除的簽到記錄的時間：", QLineEdit::Normal,"", &ok);
    QDateTime dt = QDateTime::fromString(text, "yyyy-MM-dd hh:mm:ss");
    if  (!ok || text.isEmpty() || !dt.isValid() ) {
        QMessageBox msgBox;
        msgBox.setText("請正確輸入簽到記錄的時間！");
        msgBox.exec();
    }

    QSqlQuery query;
    QString sql_str = QString("DELETE FROM signin WHERE SignIntime = '%1'").arg(text);
    if (query.exec(sql_str))
    {
        std::cout << "DELETE data successfully!" << std::endl;
        this->ShowTable();
    }
    else
    {
        std::cout << "DELETE data failed!" << std::endl;
    }
}

void MainWindow::on_SigninBtn_clicked()
{
    QString selectedSourcePath = QFileDialog::getOpenFileName(this, QObject::tr("選擇圖像"), "/home/", "Images (*.png *.jpg *.jpeg )");

    this->source_img= cv::imread(selectedSourcePath.toStdString());
    if (this->source_img.empty()) {
        ui->LoggingWindow->append(QString("[ERROR %1] %2").arg(this->currentDateTime).arg("圖像讀取失敗！"));
        return;
    }

    // Set input size before inference
    detector->setInputSize(this->source_img.size());
    cv::Mat sourceface;
    //start the detect sub-process
    emit startDetect(this->source_img);

}


