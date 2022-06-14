#ifndef CLASS_THREAD_H
#define CLASS_THREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QStringLiteral>
#include <QString>

class server_conn : public QObject
{
    Q_OBJECT
public:
    explicit server_conn(QObject *parent = nullptr);

    void server_recv(QTcpSocket *tcp);

signals:
    void recv_over();
    void openerror();
    void trans_rate(int rate);
    void conn_file(QTcpSocket *tcp);
    void recv_filename(QString filename);

private:
    QTcpSocket *m_tcp;
    bool state = true;

};

class client_conn : public QObject
{
    Q_OBJECT
public:
    explicit client_conn(QObject *parent = nullptr);

    void start_conn(QString ip, unsigned short port);
    void send_file(QString path);

signals:
    void connect_success();
    void lostconnect();
    void file_error();
    void recv_over();
    void trans_rate(int rate);

private:

    QTcpSocket * m_tcp;
    bool state = true;

};

#endif // CLASS_THREAD_H
