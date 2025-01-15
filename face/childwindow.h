#ifndef CHILDWINDOW_H
#define CHILDWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <iostream>
#include <QtSql/QSqlQueryModel>
#include <QSqlQuery>
#include <QFileDialog>
#include <QSqlTableModel>
#include <QMessageBox>

namespace Ui {
class childWindow;
}

class childWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit childWindow(QWidget *parent = nullptr);
    ~childWindow();
private:
    //創建數據庫
    void createDB();
  //創建表
    void createTable();
   //查詢
    void queryTable();
    void showTable();

private slots:
    void on_insert_btn_clicked();

    void on_modify_btn_clicked();

    void on_pushButton_clicked();

    void on_delete_btn_clicked();

private:
    Ui::childWindow *ui;
    QSqlDatabase db;
    // 保存結果
    QSqlQueryModel model;
    QString portrait_imgpath;
};

#endif // CHILDWINDOW_H
