#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_start_recv_clicked();

    void on_connect_recv_clicked();

    void on_choose_file_clicked();

    void on_send_file_clicked();

signals:

    void recv_conn(QTcpSocket * m_tcp);
    void sendfile(QString path);
    void conn_server(QString ip, unsigned short port);

private:
    Ui::Widget *ui;
    QTcpSocket * m_tcp;
    QTcpServer * m_server;
    QThread * server_thread;
    QThread * client_thread;

};
#endif // WIDGET_H
