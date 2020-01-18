#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
        connect(socket,SIGNAL(readyRead()),this,SLOT(sockReady()));
        connect(socket,SIGNAL(disconnected()),this,SLOT(sockDisc()));
    socket->connectToHost("127.0.0.1", 8080);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sockDisc()
{
    socket->deleteLater();
}

void MainWindow::addContact()
{
    if (doc.object().value("Param").toString() == "yes")
    {
        bool check = true;
        for (int i = 0; i < ui->listContact->count(); i++)
        {
            if (ui->listContact->item(i)->text() == text)
                check = false;
        }
        if (check)
            ui->listContact->addItem(text);
        else
            QMessageBox::warning(this, "Внимание","Пользователь уже добавлен");
        qDebug() << "добавили text: " << text;
    } else {
        QMessageBox::warning(this, "Внимание","Пользователя не существует. Проверьте введенные данные");
    }
}

void MainWindow::connectUser()
{
    if (doc.object().value("Param").toString() == "yes")
    {
        user0 = text;
        QMessageBox::information(this, "Добро пожаловать!","Вы успешно авторизовались");
        ui->logIn->setDisabled(true);
        ui->currLogin->clear();
        ui->currLogin->setReadOnly(true);
        ui->currLogin->setPlaceholderText("");
    } else {
        QMessageBox::warning(this, "Внимание","Логин занят");
    }
}

void MainWindow::printMessage()
{
    if (doc.object().value("Param").toString() == "ok")
    {
        qDebug()<<doc.object().value("Msg").toObject().value("UserTo").toString();
        if (doc.object().value("Msg").toObject().value("UserTo").toString() == user0)
        {
            ui->msgField->append(text);
        }
    }
}

void MainWindow::sockReady()
{
    if (socket->waitForConnected(500))
    {
        Data = socket->readAll();
        qDebug() << "прочитали Data: " << Data;
        doc = QJsonDocument::fromJson(Data, &docError);

         qDebug() << "получили ответ: " << doc.object().value("Param").toString();
         if (docError.errorString().toInt() == QJsonParseError::NoError)
         {
             if (doc.object().value("Method").toString() == "GET" && doc.object().value("Object").toString() == "contact")
                 addContact();
             if (doc.object().value("Method").toString() == "POST" && doc.object().value("Object").toString() == "contact")
                 connectUser();
             if (doc.object().value("Method").toString() == "POST" && doc.object().value("Object").toString() == "message")
                 printMessage();
         }
    }

}

void MainWindow::on_sendButton_clicked()
{
    text = ui->currMsg->text();
    std::string myString = "{\"Method\":\"POST\",\"Object\":\"message\",\"Param\":\"\", \"Msg\":{\"UserFrom\":\"" + user0.toStdString() + "\",\"UserTo\":\"" + user1.toStdString() + "\",\"Text\":\"" + text.toStdString() + "\"}}";

    if (socket->isOpen() && text.length() > 0)
    {
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();
        ui->msgField->append(text);
    }

    ui->currMsg->clear();
}


void MainWindow::on_addButton_clicked()
{

    bool ok;
    text = QInputDialog::getText(this,
                                 QString::fromUtf8("Ввод данных"),
                                 QString::fromUtf8("Введите логин пользователя:"),
                                 QLineEdit::Normal,
                                 QDir::home().dirName(), &ok);
    qDebug() << "получили text: " << text;

    // проверяем существование пользователя
    std::string myString = "{\"Method\":\"GET\",\"Object\":\"contact\",\"Param\":\"" + text.toStdString() + "\"}";

    if (socket->isOpen())
    {

        //socket->waitForBytesWritten(500);
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();

    }
}

void MainWindow::on_delButton_clicked()
{
    QListWidgetItem *it = ui->listContact->item(ui->listContact->currentRow());
    delete it;
}

void MainWindow::on_logIn_clicked()
{
    text = ui->currLogin->text();

    std::string myString = "{\"Method\":\"POST\",\"Object\":\"contact\",\"Param\":\"" + text.toStdString() + "\"}";

    if (socket->isOpen() && text.length() > 0)
    {

        //socket->waitForBytesWritten(500);
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();

    }
}

void MainWindow::on_listContact_doubleClicked(const QModelIndex &index)
{
    user1 = ui->listContact->item(index.row())->text();
    ui->msgField->clear();
    ui->msgField->setPlaceholderText("Вы можете начинать беседу с " + user1);
    //ui->sendButton->setDisabled(false);
    //ui->currMsg->setReadOnly(false);
}
