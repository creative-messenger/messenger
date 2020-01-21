#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTcpSocket* socket;
    QByteArray Data;
    QLabel *textLabel;
    QJsonDocument doc;
    QJsonParseError docError;
    QString text, user0, user1;

public slots:
    void sockReady();
    void sockDisc();
    void addContact();
    void connectUser();
    void printMessage();
    void showMessages();
    void signUpUser();
    void delContact();

private slots:
    void on_sendButton_clicked();

    void on_addButton_clicked();

    void on_delButton_clicked();

    void on_logIn_clicked();

    void on_listContact_doubleClicked(const QModelIndex &index);

    void on_signUp_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
