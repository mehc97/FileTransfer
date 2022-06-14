#include "widget.h"
#include "ui_widget.h"
#include <QThread>
#include "class_thread.h"
#include <QMessageBox>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // 设置按钮样式及悬浮、按下时的状态
    ui->connect_recv->setStyleSheet("QPushButton{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
                                    "QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
                                    "QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

    ui->choose_file->setStyleSheet("QPushButton{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
                                   "QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
                                   "QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

    ui->start_recv->setStyleSheet("QPushButton{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
                                  "QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
                                  "QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

    ui->send_file->setStyleSheet("QPushButton{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}"
                                 "QPushButton:hover{background-color:rgb(229, 241, 251); color: black;}"
                                 "QPushButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");


    //设置输入框样式
    ui->iptext->setStyleSheet("QLineEdit{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}");
    ui->porttext->setStyleSheet("QLineEdit{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}");
    ui->file_path->setStyleSheet("QLineEdit{border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}");


    //设置默认的连接ip和端口
    ui->iptext->setText("127.0.0.1");
    ui->porttext->setText("10010");

    //设置进度条的最大最小值，默认值
    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);

    //创建一个连接线程和传输线程
    server_thread = new QThread;
    client_thread = new QThread;
    //创建两个任务类，加到两个线程运行
    server_conn * server_class = new server_conn;
    client_conn * client_class = new client_conn;

    server_class->moveToThread(server_thread);
    client_class->moveToThread(client_thread);

    connect(this,&Widget::conn_server, client_class, &client_conn::start_conn);
    //主线程发出发送文件信号，任务类执行发送文件的函数
    connect(this,&Widget::sendfile, client_class, &client_conn::send_file);

    //接收子线程发出的文件路径为空的信号，做出窗口提示
    connect(client_class, &client_conn::file_error, this, [=](){
        QMessageBox::information(this, "错误提示", "发送文件不能为空！");
    });
    //主线程处理任务对象发出的信号
    connect(client_class, &client_conn::connect_success,this, [=](){
       ui->start_recv->setEnabled(false);
       ui->connect_recv->setEnabled(false);

       QMessageBox::information(this,QStringLiteral("连接提示"),QStringLiteral("连接成功"));

    });
    connect(client_class, &client_conn::lostconnect, this, [=](){

       QMessageBox::information(this,QStringLiteral("连接提示"),QStringLiteral("连接已断开"));
       client_class->deleteLater();
       client_thread->quit();
       client_thread->wait();
       client_thread->deleteLater();

       ui->connect_recv->setEnabled(true);
       ui->start_recv->setEnabled(true);

    });

    //主线程接收到子线程发的传输比例大小，进行改变进度条
    //connect(client_class, &client_conn::trans_rate, ui->progressBar, &QProgressBar::setValue);
    connect(client_class, &client_conn::trans_rate, this, [=](int rate){

        ui->progressBar->setValue(rate);
        if(rate == 100)
        {
            QMessageBox::information(this,"传输提示","传输完毕！");
            ui->choose_file->setEnabled(true);
            ui->send_file->setEnabled(true);

        }

    });
    //服务端的设置进度
    connect(server_class, &server_conn::trans_rate, this, [=](int rate){

        ui->progressBar->setValue(rate);
        if(rate == 100)
        {
            QMessageBox::information(this,"传输提示","传输完毕！");

        }

    });


    //主线程点击开始接收文件，主线程就开始监听 // //

    m_server = new QTcpServer(this);
    connect(ui->start_recv, &QPushButton::clicked, this, [=](){

        m_server->listen(QHostAddress::Any, ui->porttext->text().toUShort());

    });

    //主线程收到连接，就把通信套接字传给子线程
    connect(m_server, &QTcpServer::newConnection, this, [=](){

        m_tcp = m_server->nextPendingConnection();

        emit recv_conn(m_tcp);

        //子线程断开连接
        connect(m_tcp, &QTcpSocket::disconnected,this,[=](){
            m_tcp->close();
            m_tcp->deleteLater();
            QMessageBox::information(this,"提示","发送端断开连接");

        });

    });
    connect(this, &Widget::recv_conn, server_class, &server_conn::server_recv);

    //主线程接收到文件名，写到输入框里
    connect(server_class, &server_conn::recv_filename, this, [=](QString filename){

        ui->file_path->clear();
        ui->file_path->setText(filename);

    });


    //接收文件创建失败的信息
    connect(server_class, &server_conn::openerror, this, [=](){
        QMessageBox::information(this,"错误","文件接收失败");
    });



    //窗口退出释放资源
    connect(this, &QWidget::destroyed, this, [=](){

        m_server->close();
        m_server->deleteLater();

        server_thread->quit();
        server_thread->wait();
        server_thread->deleteLater();

        client_thread->quit();
        client_thread->wait();
        client_thread->deleteLater();
    });

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_connect_recv_clicked()
{

    client_thread->start();

    QString ip = ui->iptext->text();
    unsigned short port = ui->porttext->text().toUShort();

    emit conn_server( ip , port );
}


void Widget::on_choose_file_clicked()
{
    QString file_path = QFileDialog::getOpenFileName();
    if(file_path.isEmpty())
    {
        QMessageBox::information(this,"打开文件","打开文件为空");
        return;
    }
    ui->file_path->setText(file_path);

    qDebug()<<file_path;
}


void Widget::on_send_file_clicked()
{
    emit sendfile(ui->file_path->text());
    ui->choose_file->setEnabled(false);
    ui->send_file->setEnabled(false);
}


void Widget::on_start_recv_clicked()
{
    server_thread->start();
    ui->start_recv->setEnabled(false);
    ui->connect_recv->setEnabled(false);
    ui->choose_file->setEnabled(false);
    ui->send_file->setEnabled(false);

}

