#include "class_thread.h"

#include <QHostAddress>
#include <QFileInfo>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QTextCodec>
#include <QThread>
#include <qeventloop.h>
#include <qtimer.h>

server_conn::server_conn(QObject *parent)
    : QObject{parent}
{

}

void server_conn::server_recv(QTcpSocket *tcp)
{
    m_tcp = tcp;
    static QFile file;

    //    //接收数据
    connect(m_tcp, &QTcpSocket::readyRead, this, [=](){

       qDebug()<<"连接了";

       static qint64 num = 0;
       static qint64 filesize = 0;
       QString filename;

       QByteArray alldata = m_tcp->readAll();

       //接收到新文件，先读取文件大小
       if(state)
       {
           state = false;

           qDebug()<<alldata;

           filesize = QString(alldata).section("*",0,0).toULongLong();
           filename = QString(alldata).section("*",1,1);
           qDebug()<<filesize;
           qDebug()<<filename;

           emit recv_filename(filename);

           bool filelive = file.exists(filename);
           if(filelive)
           {
               file.remove();
           }

           file.setFileName(filename);

           bool res =file.open(QFile::WriteOnly);
           if(!res)
           {
               emit openerror();
           }

       }
       else
       {
           //QTextCodec::codecForName("gbk")->toUnicode(alldata);

           qint64 len = file.write(alldata);

           num+=len;

           qDebug()<<"接收："<<len;

           qDebug()<<"累加"<<num;

           //把进度传给主线程，主线程的进度条就可以改变
           int rate = (num*100/filesize);
           qDebug()<<"rate:"<<rate;

           emit trans_rate(rate);


           if(num == filesize)
           {
               qDebug()<<"完毕";
               file.close();
               //file->deleteLater();
               state = true;
               rate = 0;
               num = 0;

               //emit recv_over();
           }

       }

      });


}


client_conn::client_conn(QObject *parent): QObject{parent}
{

}

void client_conn::start_conn(QString ip, unsigned short port)
{
    m_tcp = new QTcpSocket;
    m_tcp->connectToHost(QHostAddress(ip),port);

    //连接成功告诉主线程
    connect(m_tcp, &QTcpSocket::connected, this, &client_conn::connect_success);
    //断开连接告诉主线程
    connect(m_tcp, &QTcpSocket::disconnected, this, [=](){
       m_tcp->close();
       m_tcp->deleteLater();
       emit lostconnect();
    });
}

void client_conn::send_file(QString path)
{
    QFile path_file(path);
    static int jianum = 1000000;

    if(path_file.open(QFile::ReadOnly))
    {
        //获取文件的名字和大小
        QFileInfo fileinfo(path_file);

        QString filename = fileinfo.fileName();
        qDebug()<<"name:"<<filename;

        qint64 filesize = fileinfo.size();
        qDebug()<<"size:"<<filesize;

        QString file = QString("%1*%2").arg(filesize).arg(filename);
        qDebug()<<file;

        qint64 num = 0;
        qint64 len = 0;

        static int rate = 0;

        char buf[50*1024] = {0};

        if(num == 0)
        {
            //第一次传输文件，先把文件大小和文件名传过去
            m_tcp->write(file.toUtf8().data());

            //qDebug()<<"utf"<<(char*)&filename;
            QEventLoop eventloop;
            QTimer::singleShot(30, &eventloop, SLOT(quit()));
            eventloop.exec();
        }

        while( num != filesize )
        {

            memset(buf,0,sizeof(buf));

            //往文件中读数据
            len = path_file.read(buf, sizeof(buf));

            //发送数据，读多少，发多少
            len = m_tcp->write(buf,len);

            qDebug()<<"发送："<<len;


            //发送的数据需要累积
            num += len;
            qDebug()<<"累加"<<num;

            //把进度传给主线程，主线程的进度条就可以改变
            rate = (num*100/filesize);
            emit trans_rate(rate);

            if(num % jianum > 1)
            {
                jianum += 1000000;
                QEventLoop eventloop;
                QTimer::singleShot(2, &eventloop, SLOT(quit()));
                eventloop.exec();
            }

        }
        if(num == filesize)
        {
            qDebug()<<"完毕";
            path_file.close();
            num = 0;
            rate = 0;
        }

    }
    else
    {
        emit file_error();
    }
}
