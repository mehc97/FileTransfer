#include "widget.h"
#include "ui_widget.h"
#include <QAxObject>
#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <cmath>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置表格多少列，行
    ui->tableWidget->setRowCount(8);
    ui->tableWidget->setColumnCount(26);

    ui->tableWidget_2->setRowCount(5);
    ui->tableWidget_2->setColumnCount(5);

    //给表头设置边框
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{padding-left: 4px;border: 1px solid #383838;}");
    ui->tableWidget_2->horizontalHeader()->setStyleSheet("QHeaderView::section{padding-left: 4px;border: 1px solid #383838;}");

    //给表2设置表头
    QStringList list;
    list << "姓名" << "缺勤时间" << "加班时间"<<"迟到次数"<<"早退次数";
        ui->tableWidget_2->setHorizontalHeaderLabels(list);

    connect(ui->inert_table,&QPushButton::clicked,this,&Widget::insert_table);
    connect(ui->start_suan,&QPushButton::clicked, this, &Widget::calculate_result);

    ui->tableWidget->setSpan(0, 0, 2, 1);
    ui->tableWidget->setSpan(0, 1, 2, 1);
    ui->tableWidget->setSpan(0, 2, 2, 1);
    ui->tableWidget->setSpan(0, 3, 1, 4);
     ui->tableWidget->setSpan(0, 7, 1, 4);
      ui->tableWidget->setSpan(0, 11, 1, 5);
       ui->tableWidget->setSpan(0, 16, 1, 5);




}

Widget::~Widget()
{
    delete ui;
}

void Widget::insert_table()
{
    QAxObject *excel = new QAxObject(this);
        excel->setControl("Excel.Application");
        excel->setProperty("Visible", false);    //显示窗体看效果,选择ture将会看到excel表格被打开
        excel->setProperty("DisplayAlerts", true);
        QAxObject *workbooks = excel->querySubObject("WorkBooks");   //获取工作簿(excel文件)集合

        filepath = QFileDialog::getOpenFileName(this,"打开",
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                   "Excel 文件(*.xls *.xlsx *.xlsm)");

        //打开刚才选定的excel
        workbooks->dynamicCall("Open(const QString&)", filepath);
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");
        QAxObject *worksheet = workbook->querySubObject("WorkSheets(int)",1);
        QAxObject *usedRange = worksheet->querySubObject("UsedRange");   //获取表格中的数据范围

        QVariant var = usedRange->dynamicCall("Value");  //将所有的数据读取刀QVariant容器中保存
        QList<QList<QVariant>> excel_list;  //用于将QVariant转换为Qlist的二维数组
        QVariantList varRows=var.toList();
        if(varRows.isEmpty())
        {
             return;
        }

        const int row_count = varRows.size();
        QVariantList rowData;
        for(int i=0;i<row_count;++i)
        {
            rowData = varRows[i].toList();
            excel_list.push_back(rowData);
        }

        qDebug()<<"row::"<<row_count;

        //打印excel数据
        for(int i = 2; i<row_count; i++)
        {
            QList<QVariant> curList = excel_list.at(i);
            int curRowCount = curList.size();
            for(int j = 0; j < curRowCount; j++)
            {
                //qDebug() << curList.at(j).toString();

                QString curstr = curList.at(j).toString();


                if(curstr.contains("T00:00:00.000", Qt::CaseSensitive))
                {
                    QString currenttime = curstr.mid(0,10);

                    //qDebug()<<"time:"<<currenttime;

                    ui->tableWidget->setItem(i-2,j,new QTableWidgetItem(currenttime));
                }
                else if(curstr.contains("0.", Qt::CaseSensitive))
                {

                    double tmpnum = curstr.toDouble();
                    allsecond =  tmpnum * 24 * 60 * 60 + 10;
                    //qDebug()<<"allsecond:"<<allsecond;

                    QTime a(0,0);
                    a = a.addSecs(int(allsecond));

                    QString strtime = a.toString("hh:mm");
                    //qDebug()<<"str:"<<strtime;

                    ui->tableWidget->setItem(i-2,j,new QTableWidgetItem(strtime));
                }
                else
                {
                    ui->tableWidget->setItem(i-2,j,new QTableWidgetItem(curList.at(j).toString()));

                }
            }
        }

        ui->tableWidget->item(0,3)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidget->item(0,7)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidget->item(0,11)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableWidget->item(0,16)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);


        workbook->dynamicCall( "Close(Boolean)", false );
        excel->dynamicCall( "Quit(void)" );
        delete excel;
}

void Widget::calculate_result()
{
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 1;

    for(i = 0; i < 5; i++)
    {
        ui->tableWidget_2->setItem(i,0, new QTableWidgetItem(ui->tableWidget->item(i+2,2)->text()));
    }

    for(i = 2; i < 7; ++i)
    {
        for(j = 4; j < 5; ++j)
        {
            qDebug()<<"time:"<<ui->tableWidget->item(i,j)->text();

            QDateTime t1time = QDateTime::fromString(ui->tableWidget->item(0,3)->text(),"yyyy-MM-dd");
            QString t1week = t1time.toString("ddd");
            if(t1week == "周六" || t1week == "周日")
            {
                chidaotime = 0;
                zaotuitime = 0;

                QString time_str = ui->tableWidget->item(i,j)->text();
                QString time_str2 = ui->tableWidget->item(i,j+1)->text();

                QTime time=QTime::fromString(time_str);
                int allsecond = time.hour()*60*60+time.minute()*60;

                QTime time2=QTime::fromString(time_str2);
                int allsecond2 = time2.hour()*60*60+time2.minute()*60;

                //算缺勤
                //大于9.00，小于18.00
                QString time_str3 = "09:00:00";
                QTime time3=QTime::fromString(time_str3);

                int allsecond3 = time3.hour()*60*60+time3.minute()*60;

                QString time_str4 = "18:00:00";
                QTime time4=QTime::fromString(time_str4);

                int allsecond4 = time4.hour()*60*60+time4.minute()*60;

                QString time_str5 = "12:00:00";
                QTime time5=QTime::fromString(time_str5);

                int allsecond5 = time5.hour()*60*60+time5.minute()*60;

//                if(allsecond > allsecond3)
//                {
//                    queqintime +=0.5;
//                }
                if(allsecond > allsecond3)
                {
                    if(allsecond > allsecond5)
                    {
                        queqintime +=3;
                    }
                    else
                    {
                        queqintime +=0.5;
                        while(1)
                        {
                            if(allsecond - 1800 > allsecond3)
                            {
                                queqintime +=0.5;
                            }
                            else
                            {
                                break;
                            }
                            allsecond -=1800;

                        }
                    }

                }


                //算加班
                QString time_str6 = "19:00";
                QTime time6=QTime::fromString(time_str6);

                int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                QString time_str7 = "13:00";
                QTime time7=QTime::fromString(time_str7);

                int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                while(allsecond5 - 1800 > allsecond)
                {
                    jiabantime+=0.5;
                }

                if(allsecond2 <= allsecond6 && allsecond2 >=allsecond7)
                {
                    jiabantime +=5;
                }

                //计算早退缺勤时间
                if(allsecond2 < allsecond4)
                {
                    if(allsecond2 > allsecond7)
                    {

                        while(allsecond4 -1800 < allsecond2)
                        {
                             queqintime += 0.5;
                        }
                    }
                    else
                    {
                         if(allsecond2 + 1800 > allsecond5)
                         {
                             queqintime +=0.5;
                         }

                         queqintime +=5;

                    }
                }

            }
            else
            {
                if(ui->tableWidget->item(i,j)->text() == NULL)
                {
                    queqintime+=0;
                }
                else
                {
                    //迟到
                    QString time_str = ui->tableWidget->item(i,j)->text(); //时分秒
                    QTime time=QTime::fromString(time_str);

                    int allsecond = time.hour()*60*60+time.minute()*60;

                   // qDebug()<<"1:"<<allsecond;

                    QString time_str2 = "09:10:00";
                    QTime time2=QTime::fromString(time_str2);

                    int allsecond2 = time2.hour()*60*60+time2.minute()*60;
                    //qDebug()<<"2:"<<allsecond2;

                    //早退
                    //qDebug()<<" yi "<<ui->tableWidget->item(i,j+1)->text();
                    QString time_str3 = ui->tableWidget->item(i,j+1)->text(); //时分秒
                    QTime time3=QTime::fromString(time_str3);

                    int allsecond3 = time3.hour()*60*60+time3.minute()*60;
                   // qDebug()<<"3:"<<allsecond3;


                    QString time_str4 = "18:00:00";
                    QTime time4=QTime::fromString(time_str4);

                    int allsecond4 = time4.hour()*60*60+time4.minute()*60;
                   // qDebug()<<"4:"<<allsecond4;


                    if(allsecond > allsecond2)
                    {
                        chidaotime++;
                    }
                    if(allsecond3 < allsecond4)
                    {
                        zaotuitime++;
                    }

                    //加班

                    QString time_str5 = "19:00";
                    QTime time5=QTime::fromString(time_str5);

                    int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                  //  qDebug()<<"5:"<<allsecond5;

                    if(allsecond > allsecond5)
                    {
                        while(allsecond - 1800 > 0)
                        {
                            jiabantime +=0.5;
                        }
                    }

                  //  qDebug()<<"jiaban:"<<jiabantime;

                    //缺勤
                    QString time_str6 = "09:00:00";
                    QTime time6=QTime::fromString(time_str6);

                    int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                 //   qDebug()<<"6:"<<allsecond6;


                    QString time_str7 = "18:00:00";
                    QTime time7=QTime::fromString(time_str7);

                    int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                    QString time_str8 = "12:00:00";
                    QTime time8=QTime::fromString(time_str8);

                    int allsecond8 = time8.hour()*60*60+time8.minute()*60;

                    QString time_str9 = "13:00";
                    QTime time9=QTime::fromString(time_str9);

                    int allsecond9 = time9.hour()*60*60+time9.minute()*60;


                    if(allsecond > allsecond6)
                    {
                        if(allsecond > allsecond8)
                        {
                            queqintime +=3;
                        }
                        else
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond - 1800 > allsecond6)
                                {
                                    queqintime +=0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond -=1800;

                            }
                        }

                    }

                    //计算早退缺勤时间
                    if(allsecond3 < allsecond4)
                    {
                        if(allsecond3 > allsecond9)
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond4 - 1800 > allsecond3)
                                {
                                    queqintime += 0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond4 -= 1800;

                            }
                        }
                        else
                        {
                             if(allsecond3 + 1800 > allsecond8)
                             {
                                 queqintime +=0.5;
                             }

                             queqintime +=5;

                        }
                    }


                //    qDebug()<<"7:"<<allsecond7;

                 //    qDebug()<<"queqin:"<<queqintime;



                }

            }

        }

//        QTime a(0,0);
//        a = a.addSecs(int(queqintime));

//        QString strtime = a.toString("hh:mm");

        ui->tableWidget_2->setItem(m, n, new QTableWidgetItem(QString::number((queqintime))));
        ui->tableWidget_2->setItem(m, n+1, new QTableWidgetItem(QString::number((jiabantime))));
        ui->tableWidget_2->setItem(m, n+2, new QTableWidgetItem(QString::number((chidaotime))));
        ui->tableWidget_2->setItem(m, n+3, new QTableWidgetItem(QString::number((zaotuitime))));

        m++;

        queqintime = 0;

        jiabantime = 0;

        chidaotime = 0;

        zaotuitime = 0;

    }


    m = 0;
    n = 1;
    queqintime = 0;

    jiabantime = 0;

    chidaotime = 0;

    zaotuitime = 0;

    for(i = 2; i < 7; ++i)
    {
        for(j = 7; j < 8; ++j)
        {
            qDebug()<<"time2:"<<ui->tableWidget->item(i,j)->text();

            QDateTime t1time = QDateTime::fromString(ui->tableWidget->item(0,7)->text(),"yyyy-MM-dd");
            QString t1week = t1time.toString("ddd");
            if(t1week == "周六" || t1week == "周日")
            {
                qDebug()<<"week:"<<t1week;

                chidaotime = 0;
                zaotuitime = 0;

                QString time_str = ui->tableWidget->item(i,j)->text();
                QString time_str2 = ui->tableWidget->item(i,j+1)->text();

                QTime time=QTime::fromString(time_str);
                int allsecond = time.hour()*60*60+time.minute()*60;

                QTime time2=QTime::fromString(time_str2);
                int allsecond2 = time2.hour()*60*60+time2.minute()*60;

                qDebug()<<"str1:"<<allsecond;
                 qDebug()<<"str2:"<<allsecond2;

                //算缺勤
                //大于9.00，小于18.00
                QString time_str3 = "09:00:00";
                QTime time3=QTime::fromString(time_str3);

                int allsecond3 = time3.hour()*60*60+time3.minute()*60;

                QString time_str4 = "18:00:00";
                QTime time4=QTime::fromString(time_str4);

                int allsecond4 = time4.hour()*60*60+time4.minute()*60;

                QString time_str5 = "12:00:00";
                QTime time5=QTime::fromString(time_str5);

                int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                qDebug()<<"1";

                if(allsecond > allsecond3)
                {
                    if(allsecond > allsecond5)
                    {
                        queqintime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond - allsecond3;
                        queqintime +=0.5;
                        while(1)
                        {
                            if(tmpnum - 1800 > 0)
                            {
                                queqintime +=0.5;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -=1800;

                        }
                    }

                }
                else if(allsecond < 0 || allsecond2 < 0)
                {
                    queqintime =0;
                }
                 qDebug()<<"2";


                //算加班
                QString time_str6 = "19:00";
                QTime time6=QTime::fromString(time_str6);

                int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                QString time_str7 = "13:00";
                QTime time7=QTime::fromString(time_str7);

                int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                if(allsecond <= allsecond5 && allsecond > 0)
                {
                    if(allsecond <= allsecond3)
                    {
                        jiabantime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond5 - allsecond;
                         jiabantime +=0.5;
                        while(1)
                        {

                            if(tmpnum - 1800 > 1800)
                            {
                                jiabantime +=0.5;
                                qDebug()<<"jiaban:"<<jiabantime;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -= 1800;
                        }
                    }
                }

                 qDebug()<<"3";

                if(allsecond2 <= allsecond6 && allsecond2 >=allsecond7)
                {
                    jiabantime +=5;
                }

                //计算早退缺勤时间
                if(allsecond2 < allsecond4)
                {
                    if(allsecond2 > allsecond7)
                    {

                        while(allsecond4 -1800 < allsecond2)
                        {
                             queqintime += 0.5;
                        }
                    }
                    else
                    {
                         if(allsecond2 + 1800 > allsecond5)
                         {
                             queqintime +=0.5;
                         }

                         queqintime +=5;

                         if(allsecond2 < 0)
                         {
                             queqintime = 0;
                         }

                    }
                }

            }
            else
            {
                if(ui->tableWidget->item(i,j)->text() == NULL)
                {
                    queqintime+=0;
                }
                else
                {
                    //迟到
                    QString time_str = ui->tableWidget->item(i,j)->text(); //时分秒
                    QTime time=QTime::fromString(time_str);

                    int allsecond = time.hour()*60*60+time.minute()*60;

                   // qDebug()<<"1:"<<allsecond;

                    QString time_str2 = "09:10:00";
                    QTime time2=QTime::fromString(time_str2);

                    int allsecond2 = time2.hour()*60*60+time2.minute()*60;
                    //qDebug()<<"2:"<<allsecond2;

                    //早退
                    //qDebug()<<" yi "<<ui->tableWidget->item(i,j+1)->text();
                    QString time_str3 = ui->tableWidget->item(i,j+1)->text(); //时分秒
                    QTime time3=QTime::fromString(time_str3);

                    int allsecond3 = time3.hour()*60*60+time3.minute()*60;
                   // qDebug()<<"3:"<<allsecond3;


                    QString time_str4 = "18:00:00";
                    QTime time4=QTime::fromString(time_str4);

                    int allsecond4 = time4.hour()*60*60+time4.minute()*60;
                   // qDebug()<<"4:"<<allsecond4;


                    if(allsecond > allsecond2)
                    {
                        chidaotime++;
                    }
                    if(allsecond3 < allsecond4)
                    {
                        zaotuitime++;
                    }

                    //加班

                    QString time_str5 = "19:00";
                    QTime time5=QTime::fromString(time_str5);

                    int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                  //  qDebug()<<"5:"<<allsecond5;

                    if(allsecond > allsecond5)
                    {
                        while(allsecond - 1800 > 0)
                        {
                            jiabantime +=0.5;
                        }
                    }

                  //  qDebug()<<"jiaban:"<<jiabantime;

                    //缺勤
                    QString time_str6 = "09:00:00";
                    QTime time6=QTime::fromString(time_str6);

                    int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                 //   qDebug()<<"6:"<<allsecond6;


                    QString time_str7 = "18:00:00";
                    QTime time7=QTime::fromString(time_str7);

                    int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                    QString time_str8 = "12:00:00";
                    QTime time8=QTime::fromString(time_str8);

                    int allsecond8 = time8.hour()*60*60+time8.minute()*60;

                    QString time_str9 = "13:00";
                    QTime time9=QTime::fromString(time_str9);

                    int allsecond9 = time9.hour()*60*60+time9.minute()*60;


                    if(allsecond > allsecond6)
                    {
                        if(allsecond > allsecond8)
                        {
                            queqintime +=3;
                        }
                        else
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond - 1800 > allsecond6)
                                {
                                    queqintime +=0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond -=1800;

                            }
                        }

                    }

                    //计算早退缺勤时间
                    if(allsecond3 < allsecond4)
                    {
                        if(allsecond3 > allsecond9)
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond4 - 1800 > allsecond3)
                                {
                                    queqintime += 0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond4 -= 1800;

                            }
                        }
                        else
                        {
                             if(allsecond3 + 1800 > allsecond8)
                             {
                                 queqintime +=0.5;
                             }

                             queqintime +=5;

                        }
                    }


                //    qDebug()<<"7:"<<allsecond7;

                 //    qDebug()<<"queqin:"<<queqintime;



                }

            }

        }

//        QTime a(0,0);
//        a = a.addSecs(int(queqintime));

//        QString strtime = a.toString("hh:mm");

        double questr = ui->tableWidget_2->item(m,n)->text().toDouble();
        queqintime += questr;

        double jiastr = ui->tableWidget_2->item(m,n+1)->text().toDouble();
        jiabantime +=jiastr;

        int chistr = ui->tableWidget_2->item(m,n+2)->text().toInt();
        chidaotime +=chistr;

        int zaostr = ui->tableWidget_2->item(m,n+3)->text().toInt();
        zaotuitime +=zaostr;

        ui->tableWidget_2->setItem(m, n, new QTableWidgetItem(QString::number((queqintime))));
        ui->tableWidget_2->setItem(m, n+1, new QTableWidgetItem(QString::number((jiabantime))));
        ui->tableWidget_2->setItem(m, n+2, new QTableWidgetItem(QString::number((chidaotime))));
        ui->tableWidget_2->setItem(m, n+3, new QTableWidgetItem(QString::number((zaotuitime))));

        m++;

        queqintime = 0;

        jiabantime = 0;

        chidaotime = 0;

        zaotuitime = 0;

    }

    m = 0;
    n = 1;
    queqintime = 0;

    jiabantime = 0;

    chidaotime = 0;

    zaotuitime = 0;

    for(i = 2; i < 7; ++i)
    {
        for(j = 12; j < 13; ++j)
        {
            qDebug()<<"time2:"<<ui->tableWidget->item(i,j)->text();

            QDateTime t1time = QDateTime::fromString(ui->tableWidget->item(0,11)->text(),"yyyy-MM-dd");
            QString t1week = t1time.toString("ddd");
            if(t1week == "周六" || t1week == "周日")
            {
                qDebug()<<"week:"<<t1week;

                chidaotime = 0;
                zaotuitime = 0;

                QString time_str = ui->tableWidget->item(i,j)->text();
                QString time_str2 = ui->tableWidget->item(i,j+1)->text();

                QTime time=QTime::fromString(time_str);
                int allsecond = time.hour()*60*60+time.minute()*60;

                QTime time2=QTime::fromString(time_str2);
                int allsecond2 = time2.hour()*60*60+time2.minute()*60;

                qDebug()<<"str1:"<<allsecond;
                 qDebug()<<"str2:"<<allsecond2;

                //算缺勤
                //大于9.00，小于18.00
                QString time_str3 = "09:00:00";
                QTime time3=QTime::fromString(time_str3);

                int allsecond3 = time3.hour()*60*60+time3.minute()*60;

                QString time_str4 = "18:00:00";
                QTime time4=QTime::fromString(time_str4);

                int allsecond4 = time4.hour()*60*60+time4.minute()*60;

                QString time_str5 = "12:00:00";
                QTime time5=QTime::fromString(time_str5);

                int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                qDebug()<<"1";

                if(allsecond > allsecond3)
                {
                    if(allsecond > allsecond5)
                    {
                        queqintime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond - allsecond3;
                        queqintime +=0.5;
                        while(1)
                        {
                            if(tmpnum - 1800 > 0)
                            {
                                queqintime +=0.5;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -=1800;

                        }
                    }

                }
                else if(allsecond < 0 || allsecond2 < 0)
                {
                    queqintime =0;
                }
                 qDebug()<<"2";


                //算加班
                QString time_str6 = "19:00";
                QTime time6=QTime::fromString(time_str6);

                int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                QString time_str7 = "13:00";
                QTime time7=QTime::fromString(time_str7);

                int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                if(allsecond <= allsecond5 && allsecond > 0)
                {
                    if(allsecond <= allsecond3)
                    {
                        jiabantime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond5 - allsecond;
                         jiabantime +=0.5;
                        while(1)
                        {

                            if(tmpnum - 1800 > 1800)
                            {
                                jiabantime +=0.5;
                                qDebug()<<"jiaban:"<<jiabantime;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -= 1800;
                        }
                    }
                }

                 qDebug()<<"3";

                if(allsecond2 <= allsecond6 && allsecond2 >=allsecond7)
                {
                    jiabantime +=5;
                }

                //计算早退缺勤时间
                if(allsecond2 < allsecond4)
                {
                    if(allsecond2 > allsecond7)
                    {

                        while(allsecond4 -1800 < allsecond2)
                        {
                             queqintime += 0.5;
                        }
                    }
                    else
                    {
                         if(allsecond2 + 1800 > allsecond5)
                         {
                             queqintime +=0.5;
                         }

                         queqintime +=5;

                         if(allsecond2 < 0)
                         {
                             queqintime = 0;
                         }

                    }
                }

            }
            else
            {
                if(ui->tableWidget->item(i,j)->text() == NULL)
                {
                    queqintime+=0;
                }
                else
                {
                    //迟到
                    QString time_str = ui->tableWidget->item(i,j)->text(); //时分秒
                    QTime time=QTime::fromString(time_str);

                    int allsecond = time.hour()*60*60+time.minute()*60;

                   // qDebug()<<"1:"<<allsecond;

                    QString time_str2 = "09:10:00";
                    QTime time2=QTime::fromString(time_str2);

                    int allsecond2 = time2.hour()*60*60+time2.minute()*60;
                    //qDebug()<<"2:"<<allsecond2;

                    //早退
                    //qDebug()<<" yi "<<ui->tableWidget->item(i,j+1)->text();
                    QString time_str3 = ui->tableWidget->item(i,j+1)->text(); //时分秒
                    QTime time3=QTime::fromString(time_str3);

                    int allsecond3 = time3.hour()*60*60+time3.minute()*60;
                   // qDebug()<<"3:"<<allsecond3;


                    QString time_str4 = "18:00:00";
                    QTime time4=QTime::fromString(time_str4);

                    int allsecond4 = time4.hour()*60*60+time4.minute()*60;
                   // qDebug()<<"4:"<<allsecond4;


                    if(allsecond > allsecond2)
                    {
                        chidaotime++;
                    }
                    if(allsecond3 < allsecond4)
                    {
                        zaotuitime++;
                    }

                    //加班

                    QString time_str5 = "19:00";
                    QTime time5=QTime::fromString(time_str5);

                    int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                  //  qDebug()<<"5:"<<allsecond5;

                    if(allsecond > allsecond5)
                    {
                        while(allsecond - 1800 > 0)
                        {
                            jiabantime +=0.5;
                        }
                    }

                  //  qDebug()<<"jiaban:"<<jiabantime;

                    //缺勤
                    QString time_str6 = "09:00:00";
                    QTime time6=QTime::fromString(time_str6);

                    int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                 //   qDebug()<<"6:"<<allsecond6;


                    QString time_str7 = "18:00:00";
                    QTime time7=QTime::fromString(time_str7);

                    int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                    QString time_str8 = "12:00:00";
                    QTime time8=QTime::fromString(time_str8);

                    int allsecond8 = time8.hour()*60*60+time8.minute()*60;

                    QString time_str9 = "13:00";
                    QTime time9=QTime::fromString(time_str9);

                    int allsecond9 = time9.hour()*60*60+time9.minute()*60;


                    if(allsecond > allsecond6)
                    {
                        if(allsecond > allsecond8)
                        {
                            queqintime +=3;
                        }
                        else
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond - 1800 > allsecond6)
                                {
                                    queqintime +=0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond -=1800;

                            }
                        }

                    }

                    //计算早退缺勤时间
                    if(allsecond3 < allsecond4)
                    {
                        if(allsecond3 > allsecond9)
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond4 - 1800 > allsecond3)
                                {
                                    queqintime += 0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond4 -= 1800;

                            }
                        }
                        else
                        {
                             if(allsecond3 + 1800 > allsecond8)
                             {
                                 queqintime +=0.5;
                             }

                             queqintime +=5;

                        }
                    }


                //    qDebug()<<"7:"<<allsecond7;

                 //    qDebug()<<"queqin:"<<queqintime;



                }

            }

        }

//        QTime a(0,0);
//        a = a.addSecs(int(queqintime));

//        QString strtime = a.toString("hh:mm");

        double questr = ui->tableWidget_2->item(m,n)->text().toDouble();
        queqintime += questr;

        double jiastr = ui->tableWidget_2->item(m,n+1)->text().toDouble();
        jiabantime +=jiastr;

        int chistr = ui->tableWidget_2->item(m,n+2)->text().toInt();
        chidaotime +=chistr;

        int zaostr = ui->tableWidget_2->item(m,n+3)->text().toInt();
        zaotuitime +=zaostr;

        ui->tableWidget_2->setItem(m, n, new QTableWidgetItem(QString::number((queqintime))));
        ui->tableWidget_2->setItem(m, n+1, new QTableWidgetItem(QString::number((jiabantime))));
        ui->tableWidget_2->setItem(m, n+2, new QTableWidgetItem(QString::number((chidaotime))));
        ui->tableWidget_2->setItem(m, n+3, new QTableWidgetItem(QString::number((zaotuitime))));

        m++;

        queqintime = 0;

        jiabantime = 0;

        chidaotime = 0;

        zaotuitime = 0;

    }

    m = 0;
    n = 1;
    queqintime = 0;

    jiabantime = 0;

    chidaotime = 0;

    zaotuitime = 0;

    QString jia_str1 = ui->tableWidget_2->item(m,n+1)->text(); //时分秒
    qDebug()<<"diyigejiaban"<<jia_str1;

    for(i = 2; i < 7; ++i)
    {
        for(j = 17; j < 18; ++j)
        {
            qDebug()<<"time2:"<<ui->tableWidget->item(i,j)->text();

            QDateTime t1time = QDateTime::fromString(ui->tableWidget->item(0,16)->text(),"yyyy-MM-dd");
            QString t1week = t1time.toString("ddd");
            if(t1week == "周六" || t1week == "周日")
            {
                qDebug()<<"week:"<<t1week;

                chidaotime = 0;
                zaotuitime = 0;

                QString time_str = ui->tableWidget->item(i,j)->text();
                QString time_str2 = ui->tableWidget->item(i,j+1)->text();

                QTime time=QTime::fromString(time_str);
                int allsecond = time.hour()*60*60+time.minute()*60;

                QTime time2=QTime::fromString(time_str2);
                int allsecond2 = time2.hour()*60*60+time2.minute()*60;

                qDebug()<<"str1:"<<allsecond;
                 qDebug()<<"str2:"<<allsecond2;

                //算缺勤
                //大于9.00，小于18.00
                QString time_str3 = "09:00:00";
                QTime time3=QTime::fromString(time_str3);

                int allsecond3 = time3.hour()*60*60+time3.minute()*60;

                QString time_str4 = "18:00:00";
                QTime time4=QTime::fromString(time_str4);

                int allsecond4 = time4.hour()*60*60+time4.minute()*60;

                QString time_str5 = "12:00:00";
                QTime time5=QTime::fromString(time_str5);

                int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                qDebug()<<"1";

                if(allsecond > allsecond3)
                {
                    if(allsecond > allsecond5)
                    {
                        queqintime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond - allsecond3;
                        queqintime +=0.5;
                        while(1)
                        {
                            if(tmpnum - 1800 > 0)
                            {
                                queqintime +=0.5;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -=1800;

                        }
                    }

                }
                else if(allsecond < 0 || allsecond2 < 0)
                {
                    queqintime =0;
                }
                 qDebug()<<"2";


                //算加班
                QString time_str6 = "19:00";
                QTime time6=QTime::fromString(time_str6);

                int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                QString time_str7 = "13:00";
                QTime time7=QTime::fromString(time_str7);

                int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                if(allsecond <= allsecond5 && allsecond > 0)
                {
                    if(allsecond <= allsecond3)
                    {
                        jiabantime +=3;
                    }
                    else
                    {
                        int tmpnum = allsecond5 - allsecond;
                         jiabantime +=0.5;
                        while(1)
                        {

                            if(tmpnum - 1800 > 1800)
                            {
                                jiabantime +=0.5;
                                qDebug()<<"jiaban:"<<jiabantime;
                            }
                            else
                            {
                                break;
                            }
                            tmpnum -= 1800;
                        }
                    }
                }

                 qDebug()<<"3";

                if(allsecond2 <= allsecond6 && allsecond2 >=allsecond7)
                {
                    jiabantime +=5;
                }

                //计算早退缺勤时间
                if(allsecond2 < allsecond4)
                {
                    if(allsecond2 > allsecond7)
                    {

                        while(allsecond4 -1800 < allsecond2)
                        {
                             queqintime += 0.5;
                        }
                    }
                    else
                    {
                         if(allsecond2 + 1800 > allsecond5)
                         {
                             queqintime +=0.5;
                         }

                         queqintime +=5;

                         if(allsecond2 < 0)
                         {
                             queqintime = 0;
                         }

                    }
                }

            }
            else
            {
                if(ui->tableWidget->item(i,j)->text() == NULL)
                {
                    queqintime+=0;
                }
                else
                {
                    //迟到
                    QString time_str = ui->tableWidget->item(i,j)->text(); //时分秒
                    QTime time=QTime::fromString(time_str);

                    int allsecond = time.hour()*60*60+time.minute()*60;

                   // qDebug()<<"1:"<<allsecond;

                    QString time_str2 = "09:10:00";
                    QTime time2=QTime::fromString(time_str2);

                    int allsecond2 = time2.hour()*60*60+time2.minute()*60;
                    //qDebug()<<"2:"<<allsecond2;

                    //早退
                    //qDebug()<<" yi "<<ui->tableWidget->item(i,j+1)->text();
                    QString time_str3 = ui->tableWidget->item(i,j+1)->text(); //时分秒
                    QTime time3=QTime::fromString(time_str3);

                    int allsecond3 = time3.hour()*60*60+time3.minute()*60;
                   // qDebug()<<"3:"<<allsecond3;


                    QString time_str4 = "18:00:00";
                    QTime time4=QTime::fromString(time_str4);

                    int allsecond4 = time4.hour()*60*60+time4.minute()*60;
                   // qDebug()<<"4:"<<allsecond4;


                    if(allsecond > allsecond2)
                    {
                        chidaotime++;
                    }
                    if(allsecond3 < allsecond4)
                    {
                        zaotuitime++;
                    }

                    //加班

                    QString time_str5 = "19:00";
                    QTime time5=QTime::fromString(time_str5);

                    int allsecond5 = time5.hour()*60*60+time5.minute()*60;

                  //  qDebug()<<"5:"<<allsecond5;

                    if(allsecond > allsecond5)
                    {
                        while(allsecond - 1800 > 0)
                        {
                            jiabantime +=0.5;
                        }
                    }

                  //  qDebug()<<"jiaban:"<<jiabantime;

                    //缺勤
                    QString time_str6 = "09:00:00";
                    QTime time6=QTime::fromString(time_str6);

                    int allsecond6 = time6.hour()*60*60+time6.minute()*60;

                 //   qDebug()<<"6:"<<allsecond6;


                    QString time_str7 = "18:00:00";
                    QTime time7=QTime::fromString(time_str7);

                    int allsecond7 = time7.hour()*60*60+time7.minute()*60;

                    QString time_str8 = "12:00:00";
                    QTime time8=QTime::fromString(time_str8);

                    int allsecond8 = time8.hour()*60*60+time8.minute()*60;

                    QString time_str9 = "13:00";
                    QTime time9=QTime::fromString(time_str9);

                    int allsecond9 = time9.hour()*60*60+time9.minute()*60;


                    if(allsecond > allsecond6)
                    {
                        if(allsecond > allsecond8)
                        {
                            queqintime +=3;
                        }
                        else
                        {
                            queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond - 1800 > allsecond6)
                                {
                                    queqintime +=0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond -=1800;

                            }
                        }

                    }

                    //计算早退缺勤时间
                    if(allsecond3 < allsecond4)
                    {
                        if(allsecond3 > allsecond9)
                        {
                            //queqintime +=0.5;
                            while(1)
                            {
                                if(allsecond4 - 1800 > allsecond3)
                                {
                                    queqintime += 0.5;
                                }
                                else
                                {
                                    break;
                                }
                                allsecond4 -= 1800;

                            }
                        }
                        else
                        {
                            while(1)
                            {
                                if(allsecond8 - 1800 > allsecond3)
                                {
                                    queqintime +=0.5;

                                }
                                else
                                {
                                    break;
                                }
                                allsecond8 -=1800;

                            }
                             if(allsecond3 + 1800 > allsecond8)
                             {
                                 queqintime +=0.5;
                             }

                             queqintime +=5;

                        }
                    }


                //    qDebug()<<"7:"<<allsecond7;

                 //    qDebug()<<"queqin:"<<queqintime;



                }

            }

        }

//        QTime a(0,0);
//        a = a.addSecs(int(queqintime));

//        QString strtime = a.toString("hh:mm");

        double questr = ui->tableWidget_2->item(m,n)->text().toDouble();
        queqintime += questr;

        double jiastr = ui->tableWidget_2->item(m,n+1)->text().toDouble();
        jiabantime +=jiastr;

        int chistr = ui->tableWidget_2->item(m,n+2)->text().toInt();
        chidaotime +=chistr;

        int zaostr = ui->tableWidget_2->item(m,n+3)->text().toInt();
        zaotuitime +=zaostr;

        ui->tableWidget_2->setItem(m, n, new QTableWidgetItem(QString::number((queqintime))));
        ui->tableWidget_2->setItem(m, n+1, new QTableWidgetItem(QString::number((jiabantime))));
        ui->tableWidget_2->setItem(m, n+2, new QTableWidgetItem(QString::number((chidaotime))));
        ui->tableWidget_2->setItem(m, n+3, new QTableWidgetItem(QString::number((zaotuitime))));

        m++;

        queqintime = 0;

        jiabantime = 0;

        chidaotime = 0;

        zaotuitime = 0;

    }


}


void Widget::on_save_file1_clicked()
{
    int i = 0;
    int j = 0;
    int m = 0;
    int n = 1;

    QAxObject *excel = new QAxObject(this);
        excel->setControl("Excel.Application");
        excel->setProperty("Visible", false);    //显示窗体看效果,选择ture将会看到excel表格被打开
        excel->setProperty("DisplayAlerts", true);
        QAxObject *workbooks = excel->querySubObject("WorkBooks");   //获取工作簿(excel文件)集合

//        filepath = QFileDialog::getSaveFileName(this,"打开",
//                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
//                                                   "Excel 文件(*.xls *.xlsx *.xlsm)");

        //打开刚才选定的excel
        workbooks->dynamicCall("Open(const QString&)", filepath);
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");
        QAxObject *worksheet = workbook->querySubObject("WorkSheets(int)",2);
        QAxObject *usedRange = worksheet->querySubObject("UsedRange");   //获取表格中的数据范围


        //!!!遍历获取数据将数据写入文件
        for (int i = 0;i < 5;i++)
        {

            for (int j = 0; j < 5; j++)
            {

                    worksheet->querySubObject("Cells(int, int)", i+7, j+1)->dynamicCall("setValue(const QVariant&)", ui->tableWidget_2->item(i, j)->text().toStdString().c_str());

            }


        }
        //!!!保存文件
        workbook->dynamicCall("SaveAs(const QString&)", QDir::toNativeSeparators(filepath));
        workbooks->dynamicCall("Close()");
        excel->dynamicCall("Quit()");

}

