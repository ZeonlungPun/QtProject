#include "mainwindow.h"
#include "./ui_mainwindow.h"
double cosine_similar_thresh = 0.363;
double l2norm_similar_thresh = 1.128;
float scoreThreshold = 0.2;
float nmsThreshold = 0.5;
float scale =1;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    FaceNet_path="/home/kingargroo/qt_projects/face/resource/face2.onnx";
    FaceDet_path="/home/kingargroo/qt_projects/face/resource/det.onnx";
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
    Imglabel=nullptr;

    //添加數據庫驅動
    db2=QSqlDatabase::addDatabase("QSQLITE");
    //設置數據庫名稱
    db2.setDatabaseName("employee_log.db");
    if (db2.open()==true)
    {
        std::cout<<"open successfully"<<std::endl;
    }
    else
    {
        std::cout<<"open failurely"<<std::endl;
    }

    this->createnewtable();




}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createnewtable()
{
    QSqlQuery query;
    QString sql_str= QString( "CREATE TABLE signin("
                              "id INT  NOT NULL,"
                              "name TEXT NOT NULL,"
                              "sign_in time TEXT NOT NULL)");

    if (query.exec(sql_str)==true)
    {
        std::cout<<"create Table successfully"<<std::endl;
    }
    else
    {
        std::cout<<"create failurely"<<std::endl;
    }
}

void MainWindow::on_pushButton_clicked()
{
    if(!childwindow_ui)
    {
        childwindow_ui = new childWindow(this);
        childwindow_ui -> show();

    }

}

void MainWindow::querrydatabase(std::string matchpath)
{
    QSqlQuery query;
    QString sql_str =QString("SELECT * FROM employee WHERE img_path ='%1'COLLATE NOCASE").arg(QString::fromStdString(matchpath));

    if (!query.exec(sql_str)) {
        std::cout << "FIND data failed: " << query.lastError().text().toStdString() << std::endl;
        return;
    }

    if (!query.next()) {
        std::cout << "No data found for the given matchpath!" << std::endl;
        return;
    }

    int id = query.value("id").toInt();
    QString name = query.value("name").toString();
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QSqlQuery insertQuery;
    QString sql_str2= QString("INSERT INTO signin VALUES(%1,'%2','%3') ").arg(id).arg(name).arg(currentDateTime);

    if (!insertQuery.exec(sql_str2)) {
        std::cout << "INSERT data failed: " << insertQuery.lastError().text().toStdString() << std::endl;
        return;
    }

    QSqlTableModel* model = new QSqlTableModel(this, db2);
    model->setTable("signin");
    model->select();

    ui->datatable->setModel(model);
    ui->datatable->resizeColumnsToContents();
    ui->datatable->resizeRowsToContents();
    ui->datatable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    std::cout << "Data displayed successfully!" << std::endl;

}


void MainWindow::on_sigin_in_btn_clicked()
{
    QString selectedSourcePath = QFileDialog::getOpenFileName(this, QObject::tr("選擇圖像"), "/home/", "Images (*.png *.jpg *.jpeg )");
    cv::Mat source_img= cv::imread(selectedSourcePath.toStdString());

    // 將 OpenCV 圖像轉換為 QImage
    QImage qImage = QImage( source_img.data,  source_img.cols,  source_img.rows,  source_img.step, QImage::Format_RGB888).rgbSwapped();

    // 將 QImage 轉換為 QPixmap
    QPixmap pixmap = QPixmap::fromImage(qImage);
    // 顯示圖像
    if (this->Imglabel == nullptr) {
         this->Imglabel = new QLabel(this->ui->displaywindow);
    }
    this->Imglabel->setPixmap(pixmap);
    this->Imglabel->resize(this->ui->displaywindow->size());
    this->Imglabel->move(0, 0);
    this->ui->displaywindow->setWidget(Imglabel);


    // Set input size before inference
    detector->setInputSize(source_img.size());
    cv::Mat sourceface,source_feature,aligned_face_source;
    detector->detect(source_img, sourceface);
    faceRecognizer->alignCrop(source_img, sourceface.row(0), aligned_face_source);
    faceRecognizer->feature(aligned_face_source, source_feature);
    source_feature = source_feature.clone();

    std::vector<float>recognize_scores;
    std::vector<std::string> file_paths;
    for (const auto& entry : fs::directory_iterator(data_path)) {
        std::string img_name = entry.path().filename().string();
        std::string img_path = data_path + "/" + img_name;
        std::cout << img_path << std::endl;

        // 加載圖像
        cv::Mat dest_img = cv::imread(img_path);
        if (dest_img.empty()) {
            std::cerr << "Failed to load image: " << img_path << std::endl;
            continue;  // 跳過當前圖片，處理下一張
        }

        cv::Mat destface, dest_feature, aligned_face_dest;

        // 檢測人臉
        detector->setInputSize(dest_img.size());
        detector->detect(dest_img, destface);
        if (destface.empty()) {
            std::cerr << "No faces detected in image: " << img_path << std::endl;
            continue;  // 跳過當前圖片，處理下一張
        }

        // 對齊和裁剪人臉
        faceRecognizer->alignCrop(dest_img, destface.row(0), aligned_face_dest);

        // 提取特徵
        faceRecognizer->feature(aligned_face_dest, dest_feature);
        dest_feature = dest_feature.clone();
        // 計算 L2 距離
        double L2_score = faceRecognizer->match(source_feature, dest_feature, cv::FaceRecognizerSF::DisType::FR_NORM_L2);
        recognize_scores.push_back(L2_score);
        file_paths.push_back(img_path);
    }
    // 使用 std::min_element 找到最小值
    auto min_it = std::min_element(recognize_scores.begin(), recognize_scores.end());

    // 計算對應的索引
    int min_index = std::distance(recognize_scores.begin(), min_it);
    if (*min_it < l2norm_similar_thresh) {
            std::cout << "Best match found at: " << file_paths[min_index] << std::endl;
            std::cout << "Matching score: " << *min_it << std::endl;
    } else {
        std::cout << "No match found below the threshold." << std::endl;
    }
    match_img_path=file_paths[min_index];
    std::cout<<"match file:"<<match_img_path<<std::endl;
    this->querrydatabase(match_img_path);

}


void MainWindow::on_path_btn_clicked()
{
    QString selectedPath = QFileDialog::getExistingDirectory(this, QObject::tr("選擇文件夾"), "/home/");
    data_path = selectedPath.toStdString();
}

