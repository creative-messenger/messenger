#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("CreativeMessenger");

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
    qDebug() << text;
    if (doc.object().value("Param").toString() == "yes")
        ui->listContact->addItem(text);
    else
        QMessageBox::warning(this, "Внимание","Пользователя не существует. Проверьте введенные данные");

}

void MainWindow::connectUser()
{
    if (doc.object().value("Param").toString() == "yes")
    {
        user0 = text.split("/")[0];
        //QMessageBox::information(this, "Добро пожаловать!","Добро пожаловать!");

        ui->logIn->setDisabled(true);
        ui->signUp->setDisabled(true);
        ui->currLogin->clear();
        ui->currLogin->setReadOnly(true);
        ui->currLogin->setPlaceholderText("");
        ui->currPass->clear();
        ui->currPass->setReadOnly(true);
        ui->currPass->setPlaceholderText("");

        int i = 0;
        while (!doc.object().value("List")[i].isUndefined())
        {
            ui->listContact->addItem(doc.object().value("List")[i].toString());
            i += 1;
        }

    } else {
        QMessageBox::warning(this, "Внимание","Логин или пароль неверный");
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

void MainWindow::showMessages()
{
    if (doc.object().value("Param").toString() == "yes")
    {
        int i = 0;
        while (!doc.object().value("List")[i].isUndefined())
        {
            ui->msgField->append(doc.object().value("List")[i].toString());
            i += 1;
        }
    }
}


void MainWindow::signUpUser()
{
    if (doc.object().value("Param").toString() == "yes")
    {
        QMessageBox::information(this, "Добро пожаловать!","Вы успешно зарегистрировались");
        ui->currPass->clear();
        ui->currLogin->clear();
    }
    else
        QMessageBox::warning(this, "Внимание","Логин занят");
}

void MainWindow::delContact()
{
    if (doc.object().value("Param").toString() == "ok")
    {
        if (user1 == ui->listContact->item(ui->listContact->currentRow())->text())
        {
            ui->msgField->clear();
            ui->msgField->setPlaceholderText("Выберите пользователя");
            ui->sendButton->setDisabled(true);
        }

        QListWidgetItem *it = ui->listContact->item(ui->listContact->currentRow());
        delete it;
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
             if (doc.object().value("Method").toString() == "POST" && doc.object().value("Object").toString() == "contact")
                 addContact();
             if (doc.object().value("Method").toString() == "POST" && doc.object().value("Object").toString() == "message")
                 printMessage();
             if (doc.object().value("Method").toString() == "GET" && doc.object().value("Object").toString() == "user")
                 connectUser();
             if (doc.object().value("Method").toString() == "GET" && doc.object().value("Object").toString() == "messages")
                 showMessages();
             if (doc.object().value("Method").toString() == "POST" && doc.object().value("Object").toString() == "user")
                 signUpUser();
             if (doc.object().value("Method").toString() == "DELETE" && doc.object().value("Object").toString() == "contact")
                 delContact();
         }
    }

}

void MainWindow::on_sendButton_clicked()
{
    text = user0 + ": " + ui->currMsg->text();
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
    bool check = true;
    for (int i = 0; i < ui->listContact->count(); i++)
    {
        if (ui->listContact->item(i)->text() == text)
            check = false;
    }
    if (check)
    {
        std::string myString = "{\"Method\":\"POST\",\"Object\":\"contact\",\"Param\":\"" + (user0 + "/" + text).toStdString() + "\"}";

        if (socket->isOpen() && text.length() > 0)
        {
            socket->write(myString.c_str());
            qDebug() << "отправили запрос: " << myString.c_str();
        }
    }
    else
        QMessageBox::warning(this, "Внимание","Пользователь уже добавлен");

}

void MainWindow::on_delButton_clicked()
{
    text = user0 + "/" + ui->listContact->item(ui->listContact->currentRow())->text();
    std::string myString = "{\"Method\":\"DELETE\",\"Object\":\"contact\",\"Param\":\"" + text.toStdString() + "\"}";

    if (socket->isOpen())
    {
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();
    }
}

void MainWindow::on_logIn_clicked()
{
    QString login = ui->currLogin->text();
    QString password = ui->currPass->text();
    text =  login + "/" + password;

    std::string myString = "{\"Method\":\"GET\",\"Object\":\"user\",\"Param\":\"" + text.toStdString() + "\"}";

    if (socket->isOpen() && login.length() > 0 && password.length() > 0)
    {
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();
    }
}

void MainWindow::on_listContact_doubleClicked(const QModelIndex &index)
{
    user1 = ui->listContact->item(index.row())->text();
    ui->msgField->clear();
    ui->msgField->setPlaceholderText("Вы можете начинать беседу с " + user1);

    std::string myString = "{\"Method\":\"GET\",\"Object\":\"messages\",\"Param\":\"" + (user0 + "/" + user1).toStdString() + "\"}";

    if (socket->isOpen())
    {
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();
    }
    ui->sendButton->setDisabled(false);
    //ui->currMsg->setReadOnly(false);
}

void MainWindow::on_signUp_clicked()
{
    QString login = ui->currLogin->text();
    QString password = ui->currPass->text();
    if (login.contains("/") || password.contains("/"))
        QMessageBox::warning(this, "Внимание","Символ '/' недопустим");
    text =  login + "/" + password;

    std::string myString = "{\"Method\":\"POST\",\"Object\":\"user\",\"Param\":\"" + text.toStdString() + "\"}";

    if (socket->isOpen() && login.length() > 0 && password.length() > 0)
    {
        socket->write(myString.c_str());
        qDebug() << "отправили запрос: " << myString.c_str();
    }
}
