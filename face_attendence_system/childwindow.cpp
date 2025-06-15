#include "childwindow.h"
#include "ui_childwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <iostream>
#include <QtSql/QSqlQueryModel>
#include <QSqlQuery>
#include <QFileDialog>
#include <QSqlTableModel>
#include <QMessageBox>
ChildWindow::ChildWindow(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::ChildWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("公司員工資料庫");
    createDB();
    createTable();
    showTable();
    std::string FaceDet_path="/home/punzeonlung/CPP/face_detection_yunet_2023mar_int8.onnx";
    detector=cv::FaceDetectorYN::create(FaceDet_path, "", cv::Size(640, 640), 0.5, 0.5, 1);
//使用 QTableView 的 clicked 或 activated 信號來偵測使用者點擊的資料行，
 //然後從資料庫中取出對應的圖片 BLOB，轉換為 QPixmap 並顯示在 QLabel（如 ui->portraitWindow）中
    connect(ui->tableWindow, &QTableView::clicked, this, &ChildWindow::onTableRowClicked);

}

void ChildWindow::onTableRowClicked(const QModelIndex &index)
{
    int row = index.row();  // 取得點擊的行

    QAbstractItemModel* model = ui->tableWindow->model();
    int id = model->data(model->index(row, 0)).toInt();  // 假設第 0 欄是 ID

    // 查詢圖片資料
    QSqlQuery query;
    query.prepare("SELECT id,name,position,gender,img FROM employee WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next())
    {
        QByteArray imageData = query.value(4).toByteArray();
        int id=query.value(0).toInt();
        QString name=query.value(1).toString();
        QString position=query.value(2).toString();
        QString gender=query.value(3).toString();

        ui->IDEdit->setText(QString::number(id));
        ui->nameEdit->setText(name);
        ui->positionBox->setCurrentText(position);
        ui->genderBox->setCurrentText(gender);


        QPixmap portraitPixmap;
        portraitPixmap.loadFromData(imageData, "PNG");  // 根據存的格式，如 PNG

        // 顯示圖片
        ui->portraitWindow->setPixmap(portraitPixmap);
        ui->portraitWindow->setScaledContents(true);
    }
}

//創建數據庫
void ChildWindow::createDB()
{
    //添加數據庫驅動
    db=QSqlDatabase::addDatabase("QSQLITE");
    //設置數據庫名稱，員工訊息表
    db.setDatabaseName("employee_log.db");
    //打開
    if (db.open()==true)
    {
        std::cout<<"create db successfully"<<std::endl;
    }
    else
    {
        std::cout<<"create db failurely"<<std::endl;
    }

}

void ChildWindow::createTable()
{
    QSqlQuery query;

    // 檢查是否已存在 employee 表
    if (query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='employee';"))
    {
        if (query.next())
        {
            std::cout << "[INFO] Table 'employee' already exists." << std::endl;
            return;
        }
    }

    // 建表語句
    QString sql_str = QString(
        "CREATE TABLE employee ("
        "id INTEGER PRIMARY KEY NOT NULL,"
                "age INTEGER NOT NULL,"
        "name TEXT NOT NULL,"
        "position TEXT NOT NULL,"
        "gender TEXT NOT NULL,"
        "img BLOB NOT NULL)"
    );

    // 執行建表
    if (query.exec(sql_str))
    {
        std::cout << "[INFO] Create table 'employee' successfully." << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] Failed to create table 'employee': "
                  << query.lastError().text().toStdString() << std::endl;
    }
}
void ChildWindow::showTable()
{

    // 創建 QSqlTableModel 來處理從數據庫中讀取數據
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("employee");
    model->select();


    // 設置表格列顯示
    ui->tableWindow->setModel(model);  // 連接模型和視圖
    ui->tableWindow->setColumnHidden(5, true);  // 隱藏 BLOB 欄
    ui->tableWindow->resizeColumnsToContents();  // 自動調整列寬
    ui->tableWindow->resizeRowsToContents();     // 自動調整行高
    QHeaderView* header = ui->tableWindow->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);  // 每列寬度相等並填滿整個表格
    // 禁用編輯模式
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

}


ChildWindow::~ChildWindow()
{
    delete ui;
}




void ChildWindow::on_insertBtn_clicked()
{
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString selectedPortraitPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇人物圖像"), "/home/", "Images (*.png *.jpg *.jpeg )");
    cv::Mat Portrait_img= cv::imread(selectedPortraitPath.toStdString());
    if (Portrait_img.empty()) {
        ui->LoggingWindow->append(QString("[ERROR %1] %2").arg(currentDateTime).arg("圖像讀取失敗！"));
        return;
    }

    // Set input size before inference
    detector->setInputSize(Portrait_img.size());
    cv::Mat sourceface,face_coordinates;

    bool faceDetected =detector->detect(Portrait_img, face_coordinates);

    sourceface=Portrait_img(cv::Rect(face_coordinates.at<float>(0, 0),face_coordinates.at<float>(0, 1),face_coordinates.at<float>(0, 2),face_coordinates.at<float>(0, 3)));

    if (!faceDetected || sourceface.empty()) {
        ui->LoggingWindow->append(QString("[WARNING %1] %2").arg(currentDateTime).arg("未檢測到人臉！"));
        return;
    }

    //將cv::mat轉化爲QImage
    QImage PortraitQImage=QImage(sourceface.data,sourceface.cols,sourceface.rows,sourceface.step,QImage::Format_RGB888).rgbSwapped();
    //    將 QImage 轉換為 QPixmap
    QPixmap Portraitpixmap = QPixmap::fromImage(PortraitQImage);
    ui->portraitWindow->setPixmap(Portraitpixmap);
    // 自動縮放圖片以適應 QLabel 大小
    ui->portraitWindow->setScaledContents(true);
    ui->LoggingWindow->append(QString("[INFO %1] %2").arg(currentDateTime).arg("簽到人頭像錄入成功！"));


    //轉成二進制儲存
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    // 將圖片以 PNG 格式寫入 byteArray
    Portraitpixmap.save(&buffer, "PNG");


    int id =ui->IDEdit->text().toInt();
    QString name=ui->nameEdit->text();
    QString position = ui->positionBox->currentText();
    QString gender = ui->genderBox->currentText();
    int age =ui->ageEdit->text().toInt();

    QSqlQuery query;
    query.prepare("INSERT INTO employee (id,age ,name, position, gender, img) "
                  "VALUES (:id,:age ,:name, :position, :gender, :img)");

    query.bindValue(":id", id);
    query.bindValue(":age", age);
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.bindValue(":gender", gender);
    query.bindValue(":img", byteArray);  // 正確插入 BLOB

    if (query.exec()==true)
    {
        std::cout<<"insert Table successfully"<<std::endl;
        this->showTable();  // 更新表格顯示
    }
    else
    {
        std::cout<<"insert failurely"<<std::endl;
    }
}

void ChildWindow::on_deleteBtn_clicked()
{
    QSqlQuery query;
    int id = ui->IDEdit->text().toInt();  // 獲取ID
    QString sql_str = QString("DELETE FROM employee WHERE id = %1").arg(id);  // 正確的SQL語句

    if (query.exec(sql_str))  // 如果執行成功
    {
        std::cout << "DELETE data successfully!" << std::endl;
        this->showTable();  // 更新表格顯示
    }
    else
    {
        std::cout << "DELETE data failed!" << std::endl;
    }
}

void ChildWindow::on_modifyBtn_clicked()
{
    QSqlQuery query;
    int id = ui->IDEdit->text().toInt();
    QString name = ui->nameEdit->text();
    QString position = ui->positionBox->currentText();
    QString gender = ui->genderBox->currentText();
    int age =ui->ageEdit->text().toInt();
    // 使用 id 作為 WHERE 條件來更新其他字段
    QString sql_str = QString("UPDATE employee SET name='%1', gender='%2', position='%3' age='%4'  WHERE id=%5")
                          .arg(name)
                          .arg(gender)
                          .arg(position)
                          .arg(age)
                          .arg(id);  // 確保 id 作為更新條件，而不是被更新

    if (query.exec(sql_str))  // 如果執行成功
    {
        std::cout << "Modify data successfully!" << std::endl;
        this->showTable();  // 更新表格顯示
    }
    else
    {
        std::cout << "Modify data failed!" << std::endl;
    }
}
