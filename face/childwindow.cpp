#include "childwindow.h"
#include "ui_childwindow.h"

childWindow::childWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::childWindow)
{
    ui->setupUi(this);
    createDB();
    createTable();
    showTable();

}

childWindow::~childWindow()
{
    delete ui;
}

//創建數據庫
void childWindow::createDB()
{
    //添加數據庫驅動
    db=QSqlDatabase::addDatabase("QSQLITE");
    //設置數據庫名稱
    db.setDatabaseName("employee_log.db");
    //打開
    if (db.open()==true)
    {
        std::cout<<"create successfully"<<std::endl;
    }
    else
    {
        std::cout<<"create failurely"<<std::endl;
    }

}
//創建表
void childWindow::createTable()
{
    QSqlQuery query;
    QString sql_str= QString( "CREATE TABLE employee("
                              "id INT PRIMARY KEY NOT NULL,"
                              "name TEXT NOT NULL,"
                              "position TEXT NOT NULL,"
                              "gender TEXT NOT NULL,"
                              "img_path TEXT NOT NULL)");
    if (query.exec(sql_str)==true)
    {
        std::cout<<"create Table successfully"<<std::endl;
    }
    else
    {
        std::cout<<"create failurely"<<std::endl;
    }

}
//查詢
void childWindow::showTable()
{
    // 創建 QSqlTableModel 來處理從數據庫中讀取數據
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("employee");    // 指定數據表
    model->select();                // 加載數據
    // 設置表格列顯示
    ui->table->setModel(model);  // 連接模型和視圖
    ui->table->resizeColumnsToContents();  // 自動調整列寬
    ui->table->resizeRowsToContents();     // 自動調整行高
    QHeaderView* header = ui->table->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);  // 每列寬度相等並填滿整個表格
    // 禁用編輯模式
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

}


void childWindow::on_insert_btn_clicked()
{
    QSqlQuery query;
    int id =ui->idEdit->text().toInt();
    QString name=ui->namemEdit->text();
    QString position = ui->positionBox->currentText();
    QString gender = ui->genderBox->currentText();
    if (portrait_imgpath.isEmpty())
    {
        QMessageBox::critical(this, "Error Message", "需要先指定肖像路徑");
        return;
    }
    QString sql_str= QString("INSERT INTO employee VALUES(%1,'%2','%3','%4','%5') ").arg(id).arg(name).arg(position).arg(gender).arg(portrait_imgpath);
    if (query.exec(sql_str)==true)
    {
        std::cout<<"insert Table successfully"<<std::endl;
        this->showTable();  // 更新表格顯示
    }
    else
    {
        std::cout<<"insert failurely"<<std::endl;
    }

}


void childWindow::on_modify_btn_clicked()
{
    QSqlQuery query;
    int id = ui->idEdit->text().toInt();
    QString name = ui->namemEdit->text();
    QString position = ui->positionBox->currentText();
    QString gender = ui->genderBox->currentText();

    // 使用 id 作為 WHERE 條件來更新其他字段
    QString sql_str = QString("UPDATE employee SET name='%1', gender='%2', position='%3' img_path='%4' WHERE id=%5")
                          .arg(name)
                          .arg(gender)
                          .arg(position)
                          .arg(portrait_imgpath)
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





void childWindow::on_pushButton_clicked()
{
    //導入人物肖像地址
   QString selectedPath = QFileDialog::getOpenFileName(this, QObject::tr("選擇圖像或者影片"), "/home/", "Images (*.png *.jpg *.jpeg *.mp4 *.avi)");
   if (!selectedPath.isEmpty())
   {
       portrait_imgpath = selectedPath;

   }
}


void childWindow::on_delete_btn_clicked()
{
    QSqlQuery query;
    int id = ui->idEdit->text().toInt();  // 獲取ID
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

