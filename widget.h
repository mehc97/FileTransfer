#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QAxObject>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void insert_table();
    void calculate_result();

private slots:


    void on_save_file1_clicked();

private:
    Ui::Widget *ui;

    //时
    int allsecond;

    //缺勤时间
    double queqintime = 0;
    //加班时间
    double jiabantime = 0;
    //迟到次数
    int chidaotime = 0;
    //早退次数
    int zaotuitime = 0;
     QString filepath;


};
#endif // WIDGET_H
