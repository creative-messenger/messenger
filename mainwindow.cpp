#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("CrMsg");
    /*QFont font("Yu Gothic UI Semilight");
    font.setPixelSize(15);
    ui->password->setFont(font);*/
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QString login = ui->login->text();
    QString password = ui->password->text();

    if (login == "Anna" && password == "test")
    {
        QMessageBox::information(this, "Ура", "Вы успешно вошли");
    }
    else
    {
        QMessageBox::information(this, "Оххх", "Вы не вошли");
    }
}

