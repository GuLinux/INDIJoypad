#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "joystickdriver.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_connectButton_clicked()
{
    qDebug() << "Connecting to INDI server" << ui->indiServer->text();
    this->indiClient = std::make_unique<INDIClient>(ui->indiServer->text());
    this->indiClient->connectServer();
}

