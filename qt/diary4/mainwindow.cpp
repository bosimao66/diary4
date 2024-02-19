﻿
/*备忘
(1)打印当前日期的年份
#include "QDate"
#include "QDebug"
 qDebug() << QDate::currentDate().year();
 某种格式
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");

 （2）定时器
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    timer->start(1000);
（3）QList的用法

例子：class QStringList : QList<QString>
成员遍历：
    while(!entrylist.isEmpty())
    {
          qDebug() << entrylist.first();
          entrylist.pop_front();
    }



*/


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QDate"
#include "QTime"
#include "QTimer"
#include "QDateTime"
#include "QFile"
#include "QDir"
#include <QMessageBox>
#include <QFileDialog>


//###################################################非此类的函数
//判断是否为日记文本名，若开头为形如 2020-10-1- 这样模式的文件名，那么默认为日记文本
static bool isDialyName(QString fileName)
{
    //有效性检测
    if(fileName.isEmpty() || fileName.length() > 100)
    {
        return false;
    }
    //至少有三个"-"
    int cnt;
     int pos;
    QString str(fileName);
    QString tmp;

    cnt = 0;
    while( str.indexOf("-")!= -1)
    {
        pos =  str.indexOf("-");
        tmp.clear();
        tmp.append(str.mid(pos + 1, str.length() - 1));
        str.clear();
        str.append(tmp);
        cnt++;

        if(cnt == 3 && str.indexOf(".") == 0)//- 和 . 相连，表示没有 title
        {
            return false;
        }

        if(cnt >= 3)break;
    }
    if(cnt < 3)
    {
        return false;
    }


    //拆解
    QString obj[4];
    str.clear();
    str.append(fileName);
    tmp.clear();
    pos = str.indexOf("-");
    obj[0].append(str.mid(0, pos));

    tmp.clear();
    tmp.append(str.mid(pos + 1, str.length() - 1));
    str.clear();
    str.append(tmp);
    pos = str.indexOf("-");
    obj[1].append(str.mid(0, pos));

    tmp.clear();
    tmp.append(str.mid(pos + 1, str.length() - 1));
    str.clear();
    str.append(tmp);
    pos = str.indexOf("-");
    obj[2].append(str.mid(0, pos));

    //解析赋值
    bool flag1, flag2, flag3;
    obj[0].toInt(&flag1, 10);
    obj[1].toInt(&flag2, 10);
    obj[2].toInt(&flag3, 10);

    if(flag1 == true && flag2 == true && flag3 == true)//都是数字，范围不定，所以不够严谨
    {
        return true;
    }

    return false;
}

//如果是日记文本名，那么解析成 年、月、日、标题和后缀名
int analyseFileName(QString fileName, int *year_p, int *month_p, int *day_p, QString *title_p, QString *type)
{
    if(isDialyName(fileName) != true)
    {
        return -1;
    }

    QString obj[5];
    int pos;
    //QString fileName("2020-10-1-title");
    QString str(fileName);
    QString tmp;
    pos = str.indexOf("-");
    obj[0].append(str.mid(0, pos));

    tmp.clear();
    tmp.append(str.mid(pos + 1, str.length() - 1));
    str.clear();
    str.append(tmp);
    pos = str.indexOf("-");
    obj[1].append(str.mid(0, pos));

    tmp.clear();
    tmp.append(str.mid(pos + 1, str.length() - 1));
    str.clear();
    str.append(tmp);
    pos = str.indexOf("-");
    obj[2].append(str.mid(0, pos));

    tmp.clear();
    tmp.append(str.mid(pos+1 , str.length() - 1));
    str.clear();
    str.append(tmp);
    pos = str.indexOf(".");
    obj[3].append(str.mid(0, pos));

    obj[4].append(str.mid(pos , str.length() - 1));//文件类型带 .

    //解析赋值,如果有一个是非数字，则返回失败，即返回-1
    bool flag1, flag2, flag3;
    *year_p = obj[0].toInt(&flag1, 10);
    *month_p = obj[1].toInt(&flag2, 10);
    *day_p = obj[2].toInt(&flag3, 10);
    if(flag1 != true || flag2 != true || flag3 != true )
    {
        return -1;
    }

    title_p->clear();
    title_p->append(obj[3]);
    type->clear();
    type->append(obj[4]);


    return 0;
}

//定制参数 排序（年月日大小排序，小在前，大在后，相等不动作）:冒泡排序
void sortByDate(int *year, int *month, int *day, int num, int *out)
{
    int i, j;
    for(i = 0; i < num ; i++)
    {
        out[i] = i;
    }


    for(i = 0; i < (num - 1); i++)
    {
        for(j = 0; j < (num - i - 1); j++)
        {
            if(year[j] > year[j + 1] ||
               (year[j] == year[j + 1] && month[j] > month[j + 1]) ||
                (year[j] == year[j + 1] && month[j] == month[j + 1] && day[j] > day[j + 1])    )
            {
                //交换位置
                out[j] ^= out[j + 1];
                out[j + 1] ^= out[j];
                out[j] ^= out[j + 1];

                year[j] ^= year[j + 1];
                year[j + 1] ^= year[j];
                year[j] ^= year[j + 1];

                month[j] ^= month[j + 1];
                month[j + 1] ^= month[j];
                month[j] ^= month[j + 1];

                day[j] ^= day[j + 1];
                day[j + 1] ^= day[j];
                day[j] ^= day[j + 1];
            }

        }
    }


}


//=========================================================================


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化定时器
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->start(1000);

    //#############################################文件浏览模块初始化
    this->ui_fileBack[0] = ui->widget;//第一个文件widget
    this->ui_fileIcon[0] = ui->btn_fileScanTmp1;
    this->ui_fileName[0] = ui->label_fileScanTmp1;
    this->var_fileObjNum = 1;//文件对象个数
    this->var_qspacerPoint = ui->verticalSpacer_2;//垂直间隔器控件指定
    this->flag_qspacerExit = 1;//垂直间隔器存在标志
    this->var_fileScanLayoutPoint = ui->verticalLayout_3;//文件浏览的布局控件指定
    this->var_scrollAreaWidgetContents = ui->scrollAreaWidgetContents;//文件浏览的 scrollArea 下幕布控件指定
    this->var_fileScanLineEditDir = ui->lineEdit_pathSet;//工作目录输入控件指定
    this->var_fileScanLabelDir = ui->label_fileScanPath;//工作目录输入提示语控件指定

   //this->var_fileScanMode = FILE_SCAN_FILE_MODE;//默认模式为文件模式
    this->var_fileScanMode = FILE_SCAN_DIALY_MODE;//默认模式为日记模式
    ui->btn_fileScanDialyMode->setStyleSheet(QLatin1String(STYLE_SHHET_BACK_COLOR_ORANGE));

    var_sheetColor[0].append(STYLE_SHHET_BACK_COLOR_RED);
    var_sheetColor[1].append(STYLE_SHHET_BACK_COLOR_ORANGE);
    var_sheetColor[2].append(STYLE_SHHET_BACK_COLOR_YELLOW);
    var_sheetColor[3].append(STYLE_SHHET_BACK_COLOR_GREEN);
    var_sheetColor[4].append(STYLE_SHHET_BACK_COLOR_CYAN);
    var_sheetColor[5].append(STYLE_SHHET_BACK_COLOR_BLUE);
    var_sheetColor[6].append(STYLE_SHHET_BACK_COLOR_PURPLE);  
    var_sheetColor[7].append(STYLE_SHHET_BACK_COLOR_WHITE);
    var_sheetColor[8].append(STYLE_SHHET_BACK_COLOR_BLACK);
    var_fileScan_dayTotalNum = 0;
    //this->fileSacnUpdate(this->var_fileScanMode, this->var_fileScanDir);//第一次更新文件浏览
    //相关槽函数
    connect(ui->btn_fileScanFileMode, &QPushButton::clicked, this, [=](){funFileScan_ModeChange(FILE_SCAN_FILE_MODE);});
    connect(ui->btn_fileScanDialyMode, &QPushButton::clicked, this, [=](){funFileScan_ModeChange(FILE_SCAN_DIALY_MODE);});
    connect(ui->btn_del, SIGNAL(clicked()), this, SLOT(fileScanFileDel()));
    connect(ui->btn_new, SIGNAL(clicked()), this, SLOT(fileScanFileAdd()));
     connect(ui->btn_fileScanToday, SIGNAL(clicked()), this, SLOT(fileScanScop_today()));
     connect(ui->btn_fileScanYestoday, SIGNAL(clicked()), this, SLOT(fileScanScop_yestoday()));
     connect(ui->btn_fileScanWeek, SIGNAL(clicked()), this, SLOT(fileScanScop_this_week()));
     connect(ui->btn_fileScanMonth, SIGNAL(clicked()), this, SLOT(fileScanScop_this_month()));

    this->var_fileScanCurrentIndex = -1;
    //ui->btn_del->hide();
    var_fileScan_scopMode = FILE_SCAN_SCOP_ALL;
    ui->calendarWidget->hide();//隐藏日历（这个版本禁用，不好看）

    //#############################################文本编辑模块初始化
    this->funEdit_init();

    //#############################其他
    //查找对话框
    //查找对话框？？
    this->findDlg = new QDialog(this);
    this->findDlg->setWindowTitle(tr("查找"));
    this->findLineEdit = new QLineEdit(findDlg);
    QPushButton *btn= new QPushButton(tr("查找下一个"), findDlg);
    QVBoxLayout *layout= new QVBoxLayout(findDlg);
    layout->addWidget(this->findLineEdit);
    layout->addWidget(btn);
    connect(btn, SIGNAL(clicked()), this, SLOT(showFindText()));
    this->varFont_size = 10;//默认字体大小
     connect(ui->action_fontAdd, &QAction::triggered, this, [=](){funOther_setFontSize(1);});
     connect(ui->action_fontSub, &QAction::triggered, this, [=](){funOther_setFontSize(2);});

    //#############################用户信息
    this->funUser_init();

    //############################云端同步功能
    cloudInit();
}

MainWindow::~MainWindow()
{
    this->show();
    this->funEditAction_closeAll();//关闭文档再结束程序

    delete ui;
}




//#############################################文件浏览页面模块
//浏览更新
void MainWindow::fileSacnUpdate(int mode, QString dir)
{
    int static flag_cnt = 0;//调用次数的标志
    if(flag_cnt == 1)
    {
        this->var_fileScanLabelDir->setText("路径");//设置工作目录 提示标签
    }

    this->var_fileScanLineEditDir->setText(dir);

    if(mode == FILE_SCAN_FILE_MODE)
    {
         this->fileSacnUpdate_fileMode(dir);
    }
    else if(mode == FILE_SCAN_DIALY_MODE)
    {

        this->fileSacnUpdate_dialyMode(dir);
    }

    flag_cnt ++;
}

//日记模式
void MainWindow::fileSacnUpdate_dialyMode(QString dir)
{
    QDir qdir;
    qdir.cd(dir);
    QStringList fileList = qdir.entryList(QDir::Files, QDir::NoSort);
    QString fileName, btnText, labelText;
    this->fileScanClear();

    int i, j, index, flag;
    int yearArray[VAR_FILE_SCAN_MAX_ENTRY], monthArray[VAR_FILE_SCAN_MAX_ENTRY], dayArray[VAR_FILE_SCAN_MAX_ENTRY];
    QString strArray[VAR_FILE_SCAN_MAX_ENTRY];
    QString titleArray[VAR_FILE_SCAN_MAX_ENTRY];
    int cnt = 0;
    while( !fileList.isEmpty() )
    {
        fileName.clear();
        fileName.append(fileList.first());
        fileList.pop_front();

        //获取第几天，index 表示
        //注：日记文件同一天归为一个集合，不同的月+日 分配不同的序号
        int year, month, day;
        QString title, type;
        if(0 != analyseFileName(fileName, &year, &month, &day, &title, &type ))
        {
            //解析失败 ,跳过
            continue;
        }

        //范围检测（以系统时间为基准）
        QDate qdate;
        QDate tmpdata(year, month, day);
        int dayOfyear = tmpdata.dayOfYear();

        if(var_fileScan_scopMode == FILE_SCAN_SCOP_ALL)
        {
            ;
        }
        else if(var_fileScan_scopMode == FILE_SCAN_SCOP_TODAY)
        {
            if(year != qdate.currentDate().year() || month != qdate.currentDate().month()
               || day != qdate.currentDate().day() )
            {
                continue;
            }
        }
        else if(var_fileScan_scopMode == FILE_SCAN_SCOP_YESTODAY)
        {

            if(year != qdate.currentDate().year() || month != qdate.currentDate().month()
               || dayOfyear != (qdate.currentDate().dayOfYear() - 1) )
            {
                continue;
            }
        }
        else if(var_fileScan_scopMode == FILE_SCAN_SCOP_THIS_WEEK)
        {
            int weekDay = qdate.currentDate().dayOfWeek();
            int yearDay = qdate.currentDate().dayOfYear();
            if(year != qdate.currentDate().year() || month != qdate.currentDate().month()
               || (dayOfyear <= (yearDay - weekDay) || dayOfyear > (yearDay + 7 - weekDay)))
            {
                continue;
            }
        }
        else if(var_fileScan_scopMode == FILE_SCAN_SCOP_THIS_MOTH)
        {

            if(year != qdate.currentDate().year() || month != qdate.currentDate().month() )
            {
                continue;
            }
        }
        else if(var_fileScan_scopMode == FILE_SCAN_SCOP_ASSIGN_DAY)
        {

            if(this->varCalendar_dayOfYear != dayOfyear )
            {
                continue;
            }
        }


        yearArray[cnt] = year, monthArray[cnt] = month, dayArray[cnt] = day;
        strArray[cnt].clear();strArray[cnt].append(fileName);
        titleArray[cnt].clear();titleArray[cnt].append(title);
        cnt++;
     }
#if 0
    qDebug()<<"=============================测试1";
    for(i = 0; i < cnt; i++)
    {
        qDebug()<<"i: "<<i<<" year:"<<yearArray[i]<<" month:"<<monthArray[i]<<" day:"<<dayArray[i]<<" str:"<<strArray[i];
    }

    int sort[cnt];
    sortByDate(yearArray, monthArray, dayArray, cnt, sort);
    qDebug()<<"=============================测试2";
    for(i = 0; i < cnt; i++)
    {
        qDebug()<<"i: "<<i<<" year:"<<yearArray[i]<<" month:"<<monthArray[i]<<" day:"<<dayArray[i]<<" str:"<<strArray[sort[i]]<<"title:"<<titleArray[sort[i]];
    }

#else
    int sort[cnt];
    sortByDate(yearArray, monthArray, dayArray, cnt, sort);

     for(j = 0; j < cnt; j++)
     {
        //qDebug()<<"j: "<<j<<" year:"<<yearArray[j]<<" month:"<<monthArray[j]<<" day:"<<dayArray[j]<<" str:"<<strArray[sort[j]]<<"title:"<<titleArray[sort[j]];
        //根据 月和日 来设置按钮颜色；问题：只有增加，没有删除
        flag = 0;
        for(i = 0; i < this->var_fileScan_dayTotalNum; i++)
        {
            if(monthArray[j] == this->var_fileScan_dateInfo[i].month && dayArray[j] == this->var_fileScan_dateInfo[i].day)
            {
                index = i;
                flag = 1;
                break;
            }
        }
        if(flag == 0)
        {
            index = this->var_fileScan_dayTotalNum;
            this->var_fileScan_dateInfo[index].month = monthArray[j];
            this->var_fileScan_dateInfo[index].day = dayArray[j];
            this->var_fileScan_dayTotalNum++;

        }
        //====================================================

        btnText.sprintf("%d/%d", monthArray[j], dayArray[j]);
        labelText.clear();
        labelText.append(titleArray[sort[j]]);
        //不同天数颜色不同
        this->fileScanCreateEntry(var_sheetColor[index % 6], btnText, QString(STYLE_SHHET_BACK_COLOR_WHITE), labelText);
        this->varFileScan_fileName[this->var_fileObjNum - 1].clear();
        this->varFileScan_fileName[this->var_fileObjNum - 1].append(strArray[sort[j]]);//把文件名存入类成员

        this->var_fileScanLayoutPoint->addWidget(this->ui_fileBack[this->var_fileObjNum -1]);

        if(this->var_fileObjNum >= VAR_FILE_SCAN_MAX_ENTRY)
        {
            this->var_fileObjNum --;
            throw QString("var_qwidgetPoint is error");
            break;
        }

    }
#endif
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    this->var_fileScanLayoutPoint->addItem(verticalSpacer);
    this->var_qspacerPoint = verticalSpacer;
    this->flag_qspacerExit = 1;


}

//文件模式
void MainWindow::fileSacnUpdate_fileMode(QString dir)
{
    QDir qdir;
    qdir.cd(dir);
    QStringList fileList = qdir.entryList(QDir::Files, QDir::NoSort);
    QStringList floderList = qdir.entryList(QDir::Dirs, QDir::NoSort);
    //qDebug()<< "文件"<<fileList;
    //qDebug()<< "文件夹"<<floderList;
    QString name;
    QString btnText;
    this->fileScanClear();

    while( !floderList.isEmpty() )
    {
        /*if(QString(".").compare(floderList.first()) == 0 ||
           QString("..").compare(floderList.first()) == 0 )//忽略目录"."和".."
        {
            floderList.pop_front();
            continue;
        }*/
        name.clear();
        name.append(floderList.first());
        floderList.pop_front();
        this->fileScanCreateEntry(STYLE_SHHET_BACK_COLOR_ORANGE, btnText, STYLE_SHHET_BACK_COLOR_WHITE, name);
        this->type_fileType[this->var_fileObjNum - 1] = FILE_SCAN_FILETYPE_FLODER;
        this->var_fileScanLayoutPoint->addWidget(this->ui_fileBack[this->var_fileObjNum -1]);

        if(this->var_fileObjNum >= VAR_FILE_SCAN_MAX_ENTRY)
        {
            this->var_fileObjNum --;
            throw QString("var_qwidgetPoint is error");
            break;
        }


    }

   while( !fileList.isEmpty() )
    {
       name.clear();
        name.append(fileList.first());
        fileList.pop_front();
        this->fileScanCreateEntry(STYLE_SHHET_BACK_COLOR_GREEN, btnText, STYLE_SHHET_BACK_COLOR_WHITE, name);
        this->type_fileType[this->var_fileObjNum -1] = FILE_SCAN_FILETYPE_FILE;
        this->varFileScan_fileName[this->var_fileObjNum - 1].append(name);//把文件名存入类成员
        this->var_fileScanLayoutPoint->addWidget(this->ui_fileBack[this->var_fileObjNum -1]);

        if(this->var_fileObjNum >= VAR_FILE_SCAN_MAX_ENTRY)
        {
            this->var_fileObjNum --;
            throw QString("var_qwidgetPoint is error");
            break;
        }

    }

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    this->var_fileScanLayoutPoint->addItem(verticalSpacer);
    this->var_qspacerPoint = verticalSpacer;
    this->flag_qspacerExit = 1;

}

//内容清除
void MainWindow::fileScanClear()
{

    //注：var_qwidgetPoint[i]中i的数值不能大于设定的VAR_FILE_SCAN_MAX_ENTRY，
    //否则指针指向未定义处，程序崩溃。
    int i;
    for(i = 0; i < this->var_fileObjNum; i++)
    {
        if(i >= VAR_FILE_SCAN_MAX_ENTRY)
        {
            throw QString("var_qwidgetPoint is error");
            break;
        }
        //ui->verticalLayout->removeWidget(this->var_qwidgetPoint[i]);
        delete this->ui_fileBack[i];
    }

    this->var_fileObjNum = 0;
    if(this->flag_qspacerExit == 1)
    {
        this->var_fileScanLayoutPoint->removeItem(this->var_qspacerPoint);
        this->flag_qspacerExit = 0;
    }
}

//构建文件显示ui
void MainWindow::fileScanCreateEntry(QString btnColor, QString btnText, QString labelColor, QString labelText)
{

    QWidget *widget = new QWidget(this->var_scrollAreaWidgetContents);
    //widget->setObjectName(QStringLiteral("widget"));
    widget->setMaximumSize(QSize(240, 35));

    QHBoxLayout *horizontalLayout = new QHBoxLayout(widget);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);
    //horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout->setContentsMargins(2, 2, 2, 2);

    QPushButton *pushButton = new QPushButton(widget);
    //pushButton->setObjectName(QStringLiteral("pushButton_2"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumSize(QSize(32, 32));
    pushButton->setMaximumSize(QSize(32, 32));
    pushButton->setStyleSheet(QLatin1String(btnColor.toLatin1()));
    pushButton->setText(btnText);


    horizontalLayout->addWidget(pushButton);
    QLabel *label = new QLabel(widget);
    //label->setObjectName(QStringLiteral("label"));
    label->setStyleSheet(labelColor.toLatin1());
    label->setText(labelText);
    horizontalLayout->addWidget(label);

    this->ui_fileBack[this->var_fileObjNum] = widget;
    this->ui_fileIcon[this->var_fileObjNum] = pushButton;
    this->ui_fileName[this->var_fileObjNum] = label;
    //this->type_fileType[this->var_fileObjNum] = type;
    this->var_fileObjNum ++;
    //创建槽
    int tmp = this->var_fileObjNum;
    connect(pushButton, &QPushButton::clicked, this, [=](){fileButtonOnclick(tmp - 1);});

}

//文件按钮点击响应
void MainWindow::fileButtonOnclick(int index)
{
    qDebug() << "fileButtonOnclick is comming" << index;
    QDir qdir;
    qdir.setPath(this->var_fileScanDir);
    if(this->var_fileScanMode == FILE_SCAN_FILE_MODE)
    {

        if(this->type_fileType[index] == FILE_SCAN_FILETYPE_FLODER)//打开文件夹
        {
            QString floderName;
            floderName.clear();
            //floderName.append(".\\");
            floderName.append(this->ui_fileName[index]->text());


            if(false == qdir.cd(floderName))
            {
                qDebug()<<"MainWindow::fileButtonOnclick: file open faild";
                return;
            }
            this->var_fileScanDir.clear();
            this->var_fileScanDir.append(qdir.path());
            this->fileSacnUpdate(FILE_SCAN_FILE_MODE, this->var_fileScanDir);
            return;
        }
        else if(this->type_fileType[index] == FILE_SCAN_FILETYPE_FILE)//打开 .txt文件
        {
            QString fileName;
            fileName.clear();
            fileName.append(this->ui_fileName[index]->text());

            if(isDialyName(fileName) == true)
            {
                this->funEdit_fileOpen(EDIT_FILE_OPEN_ANALYSE, this->var_fileScanDir, fileName);
            }
            else
            {
                this->funEdit_fileOpen(EDIT_FILE_OPEN_GENERAL, this->var_fileScanDir, fileName);
                qDebug()<<"fileButtonOnclick: open "<<this->var_fileScanDir<<fileName;
            }


        }

    }
    else if(this->var_fileScanMode == FILE_SCAN_DIALY_MODE)
    {

        this->funEdit_fileOpen(EDIT_FILE_OPEN_ANALYSE, this->var_fileScanDir, this->varFileScan_fileName[index]);

        int i;
        for(i = 0; i < this->var_fileObjNum; i++)
        {
            if(i == index)
            {
                this->ui_fileName[i]->setStyleSheet(STYLE_SHHET_BACK_COLOR_YELLOW);
            }
            else
            {
                this->ui_fileName[i]->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
            }
        }

        this->var_fileScanCurrentIndex = index;
        this->var_fileScanCurrentFileName.clear();
        this->var_fileScanCurrentFileName.append(this->varFileScan_fileName[index]);
    }

}


//日历点击事件
void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
    //date.toString("yyyy-MM-dd");
    //qDebug() <<"日期："<<date.toString("yyyy-MM-dd");

    this->varCalendar_dayOfYear = date.dayOfYear();
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_ASSIGN_DAY;
    this->funFileScan_ModeChange(FILE_SCAN_DIALY_MODE);

    ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);


}

//日记范围-全部
void MainWindow::on_btn_fileScanAll_clicked()
{
    //this->flag_fileScanRange = FILE_SCAN_RANGE_ALL;


//#define FILE_SCAN_SCOP_TODAY 1
//#define FILE_SCAN_SCOP_YESTODAY 2
//#define FILE_SCAN_SCOP_THIS_WEEK 3
//#define FILE_SCAN_SCOP_THIS_MOTH 4
//#define FILE_SCAN_SCOP_ALL 5
    if(this->var_fileScanMode != FILE_SCAN_DIALY_MODE) return;
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_ALL;
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

    ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
    ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
}

void MainWindow::fileScanScop_today()
{
     if(this->var_fileScanMode != FILE_SCAN_DIALY_MODE) return;
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_TODAY;
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

     ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
     ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
      ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);

}
void MainWindow::fileScanScop_yestoday()
{
     if(this->var_fileScanMode != FILE_SCAN_DIALY_MODE) return;
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_YESTODAY;
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

     ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
     ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
      ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);

}
void MainWindow::fileScanScop_this_week()
{
     if(this->var_fileScanMode != FILE_SCAN_DIALY_MODE) return;
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_THIS_WEEK;
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

     ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
     ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
      ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
}
void MainWindow::fileScanScop_this_month()
{
     if(this->var_fileScanMode != FILE_SCAN_DIALY_MODE) return;
    this->var_fileScan_scopMode = FILE_SCAN_SCOP_THIS_MOTH;
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

     ui->btn_fileScanToday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanYestoday->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanWeek->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
     ui->btn_fileScanMonth->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
      ui->btn_fileScanAll->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
}


//工作目录设置
void MainWindow::on_btn_pathSet_clicked()
{


    this->app_work_path.clear();
    QDir qdir;
    if(this->var_fileScanLineEditDir->text().isEmpty() ||
       qdir.cd(this->var_fileScanLineEditDir->text()) == false   )//假如为空或目录不存在，则选择默认路径
    {
        this->app_work_path.append(this->var_userDefaultDir);
        this->var_fileScanLineEditDir->setText(this->var_userDefaultDir);
    }
    else
    {
         this->app_work_path.append(this->var_fileScanLineEditDir->text());
    }

    this->fileSacnUpdate(this->var_fileScanMode, this->app_work_path);
}


//浏览模式更改
void MainWindow::funFileScan_ModeChange(int mode)
{
    if(mode == FILE_SCAN_FILE_MODE /*&& this->var_fileScanMode != FILE_SCAN_FILE_MODE*/)
    {
        this->var_fileScanMode = FILE_SCAN_FILE_MODE;//默认模式为文件模式
        ui->btn_fileScanFileMode->setStyleSheet(QLatin1String(STYLE_SHHET_BACK_COLOR_ORANGE));
        ui->btn_fileScanDialyMode->setStyleSheet(QLatin1String(STYLE_SHHET_BACK_COLOR_WHITE));
        this->fileSacnUpdate(FILE_SCAN_FILE_MODE, this->var_fileScanDir);

        ui->btn_del->hide();
        ui->btn_new->hide();
    }
    else if(mode == FILE_SCAN_DIALY_MODE /*&& this->var_fileScanMode != FILE_SCAN_DIALY_MODE*/)
    {
        this->var_fileScanMode = FILE_SCAN_DIALY_MODE;//默认模式为文件模式
        ui->btn_fileScanDialyMode->setStyleSheet(QLatin1String(STYLE_SHHET_BACK_COLOR_ORANGE));
        ui->btn_fileScanFileMode->setStyleSheet(QLatin1String(STYLE_SHHET_BACK_COLOR_WHITE));
        this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, this->var_fileScanDir);

         ui->btn_del->show();
         ui->btn_new->show();
    }


}


void MainWindow::fileScanFileDel()
{
    //delete this->ui_tabWidget[this->var_fileScanCurrentIndex];

    QString path(this->var_fileScanDir);
    path.append("/");
    path.append(this->var_fileScanCurrentFileName);
    QFile qfile(path);
    if(false == qfile.open(QIODevice::ReadOnly))
    {
        return ;
    }
    else
    {
        //再次判断是否删除
        QMessageBox::StandardButton  result=QMessageBox::question(this, "删除", "确定要删除吗？",
                          QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                          QMessageBox::NoButton); //缺省是no button
        if(result == QMessageBox::Yes)
        {
            ;

        }
        else if(result == QMessageBox::No)
        {
            return;
        }
        else
        {
            return;
        }

    }


    qfile.remove();
     //记录保存
    cloud_file_int->fileRecord_delFile(this->var_fileScanCurrentFileName);


    //清除tab
    TabLink *prob = this->link_method.fun_fileExit(this->link_head, this->var_fileScanDir, this->var_fileScanCurrentFileName);
    int index = prob->fun_getIndex(this->link_head);
    if(NULL != prob)//找到了
    {
        this->funEdit_tabDel(EDIT_TAB_DEL_MODE_BYINDEX, index);
        qDebug()<<"funEditAction_new tip: file tab is deleted!";

    }

    this->funFileScan_ModeChange(FILE_SCAN_DIALY_MODE);

    ui->btn_del->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
    ui->btn_new->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);



}

void MainWindow::fileScanFileAdd()
{
    this->funEditAction_new();

    this->funFileScan_ModeChange(FILE_SCAN_DIALY_MODE);
    ui->btn_new->setStyleSheet(STYLE_SHHET_BACK_COLOR_ORANGE);
     ui->btn_del->setStyleSheet(STYLE_SHHET_BACK_COLOR_WHITE);
}

//===========================================================








//#############################################文本操作模块
//初始化
void MainWindow::funEdit_init()
{


    this->varEdit_weekStr[1].append("一");
    this->varEdit_weekStr[2].append("二");
    this->varEdit_weekStr[3].append("三");
    this->varEdit_weekStr[4].append("四");
    this->varEdit_weekStr[5].append("五");
    this->varEdit_weekStr[6].append("六");
    this->varEdit_weekStr[7].append("日");
    //新建、打开、保存、另存为 的槽
    connect(ui->action_new, SIGNAL(triggered()), this, SLOT(funEditAction_new()));
    connect(ui->action_open, SIGNAL(triggered()), this, SLOT(funEditAction_open()));
    connect(ui->action_save, SIGNAL(triggered()), this, SLOT(funEditAction_save()));
    connect(ui->action_saveAll, SIGNAL(triggered()), this, SLOT(funEditAction_saveAll()));
    connect(ui->action_saveAs, SIGNAL(triggered()), this, SLOT(funEditAction_saveAs()));
    connect(ui->action_close, SIGNAL(triggered()), this, SLOT(funEditAction_close()));
    connect(ui->action_closeAll, SIGNAL(triggered()), this, SLOT(funEditAction_closeAll()));
    connect(ui->action_closeOther, SIGNAL(triggered()), this, SLOT(funEditAction_closeOther()));



    //ui 控件
    this->ui_tabWidget = ui->tabWidget;
    //链表
    this->link_head = this->link_method.fun_create();


    //当前ui界面的tab不属于链表里的一员，先把它删掉
    ui->tabWidget->removeTab(0);
    //然后打开一个属于链表的tab，相当于新建按钮的一次动作
    this->funEdit_tabAdd(EDIT_TAB_ADD_MODE_NEW, this->var_fileScanDir, "new");
}


//tab界面对象创建
void MainWindow::funEdit_tabWidgetCreate(TabLink *node)
{
    //绘制UI界面
    QWidget *widget = new QWidget();
    widget->setObjectName(QStringLiteral("widget"));
    widget->setGeometry(QRect(40, 40, 518, 298));
    widget->setMinimumSize(QSize(502, 0));
    widget->setMaximumSize(QSize(802, 16777215));
    QVBoxLayout *verticalLayout = new QVBoxLayout(widget);
    verticalLayout->setSpacing(0);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    QLineEdit *lineEdit_title = new QLineEdit(widget);
    lineEdit_title->setObjectName(QStringLiteral("lineEdit_title"));
    lineEdit_title->setMinimumSize(QSize(500, 0));
    lineEdit_title->setMaximumSize(QSize(800, 16777215));
    QFont font;
    font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
    font.setPointSize(13);
    lineEdit_title->setFont(font);

    verticalLayout->addWidget(lineEdit_title);

    QFrame *frame_3 = new QFrame(widget);
    frame_3->setObjectName(QStringLiteral("frame_3"));
    frame_3->setMinimumSize(QSize(500, 0));
    frame_3->setMaximumSize(QSize(800, 40));
    frame_3->setFrameShape(QFrame::StyledPanel);
    frame_3->setFrameShadow(QFrame::Raised);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(frame_3);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout->setContentsMargins(10, 0, 10, 0);
    QLabel *label_time = new QLabel(frame_3);
    label_time->setObjectName(QStringLiteral("label_time"));
    label_time->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_time);

    QLineEdit *lineEdit_dateYear = new QLineEdit(frame_3);
    lineEdit_dateYear->setObjectName(QStringLiteral("lineEdit_dateYear"));
    lineEdit_dateYear->setMaximumSize(QSize(35, 16777215));

    horizontalLayout->addWidget(lineEdit_dateYear);

    QLabel *label_year = new QLabel(frame_3);
    label_year->setObjectName(QStringLiteral("label_year"));
    label_year->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_year);

    QLineEdit *lineEdit_dateMonth = new QLineEdit(frame_3);
    lineEdit_dateMonth->setObjectName(QStringLiteral("lineEdit_dateMonth"));
    lineEdit_dateMonth->setMaximumSize(QSize(25, 16777215));

    horizontalLayout->addWidget(lineEdit_dateMonth);

    QLabel *label_mounth = new QLabel(frame_3);
    label_mounth->setObjectName(QStringLiteral("label_mounth"));
    label_mounth->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_mounth);

    QLineEdit *lineEdit_dateDay = new QLineEdit(frame_3);
    lineEdit_dateDay->setObjectName(QStringLiteral("lineEdit_dateDay"));
    lineEdit_dateDay->setMaximumSize(QSize(25, 16777215));

    horizontalLayout->addWidget(lineEdit_dateDay);

    QLabel *label_day = new QLabel(frame_3);
    label_day->setObjectName(QStringLiteral("label_day"));
    label_day->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_day);

    QLineEdit *lineEdit_timeHour = new QLineEdit(frame_3);
    lineEdit_timeHour->setObjectName(QStringLiteral("lineEdit_timeHour"));
    lineEdit_timeHour->setMaximumSize(QSize(25, 16777215));

    horizontalLayout->addWidget(lineEdit_timeHour);

    QLabel *label_hour = new QLabel(frame_3);
    label_hour->setObjectName(QStringLiteral("label_hour"));
    label_hour->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_hour);

    QLineEdit *lineEdit_timeMinute = new QLineEdit(frame_3);
    lineEdit_timeMinute->setObjectName(QStringLiteral("lineEdit_timeMinute"));
    lineEdit_timeMinute->setMaximumSize(QSize(25, 16777215));

    horizontalLayout->addWidget(lineEdit_timeMinute);

    QLabel *label_minute = new QLabel(frame_3);
    label_minute->setObjectName(QStringLiteral("label_minute"));
    label_minute->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_minute);

    QSpacerItem *horizontalSpacer_tab1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_tab1);

    QLabel *label_weather = new QLabel(frame_3);
    label_weather->setObjectName(QStringLiteral("label_weather"));
    label_weather->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_weather);

    QLineEdit *lineEdit_weather = new QLineEdit(frame_3);
    lineEdit_weather->setObjectName(QStringLiteral("lineEdit_weather"));
    lineEdit_weather->setMaximumSize(QSize(60, 16777215));

    horizontalLayout->addWidget(lineEdit_weather);

    QSpacerItem *horizontalSpacer_tab2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_tab2);

    QLabel *label_week = new QLabel(frame_3);
    label_week->setObjectName(QStringLiteral("label_week"));
    label_week->setMaximumSize(QSize(40, 16777215));

    horizontalLayout->addWidget(label_week);

    QLineEdit *lineEdit_week = new QLineEdit(frame_3);
    lineEdit_week->setObjectName(QStringLiteral("lineEdit_week"));
    lineEdit_week->setMaximumSize(QSize(30, 16777215));

    horizontalLayout->addWidget(lineEdit_week);

    horizontalLayout->setStretch(0, 2);
    horizontalLayout->setStretch(1, 2);
    horizontalLayout->setStretch(2, 1);
    horizontalLayout->setStretch(3, 1);
    horizontalLayout->setStretch(4, 1);
    horizontalLayout->setStretch(5, 1);
    horizontalLayout->setStretch(6, 1);
    horizontalLayout->setStretch(7, 1);
    horizontalLayout->setStretch(8, 1);
    horizontalLayout->setStretch(9, 1);
    horizontalLayout->setStretch(10, 1);
    horizontalLayout->setStretch(11, 4);
    horizontalLayout->setStretch(12, 2);
    horizontalLayout->setStretch(13, 6);
    horizontalLayout->setStretch(14, 4);
    horizontalLayout->setStretch(15, 2);
    horizontalLayout->setStretch(16, 2);

    verticalLayout->addWidget(frame_3);

    QTextEdit *textEdit_mainBody = new QTextEdit(widget);
    textEdit_mainBody->setObjectName(QStringLiteral("textEdit_mainBody"));
    textEdit_mainBody->setMinimumSize(QSize(500, 0));
    textEdit_mainBody->setMaximumSize(QSize(800, 16777215));
    textEdit_mainBody->setTabStopWidth(32);

    font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
    if(this->varFont_size >= 9 && this->varFont_size <= 20)
    {
         font.setPointSize(this->varFont_size);
    }
    else
    {
        font.setPointSize(12);
    }

    textEdit_mainBody->setFont(font);
    textEdit_mainBody->setAcceptRichText(false);

    verticalLayout->addWidget(textEdit_mainBody);

    QLabel *label_statusBar = new QLabel(widget);
    label_statusBar->setObjectName(QStringLiteral("label_statusBar"));
    label_statusBar->setMinimumSize(QSize(500, 0));
    label_statusBar->setMaximumSize(QSize(800, 16777215));

    verticalLayout->addWidget(label_statusBar);

    label_time->setText("时间：");
    label_year->setText("年");
    label_mounth->setText("月");
    label_day->setText("日");
    label_hour->setText("时");
    label_minute->setText("分");
    label_weather->setText("天气：");
    label_week->setText("星期：");
    label_statusBar->setText("状态栏");


    //添加进链表
    node->uiTab  = widget;//指向文本编辑对象所在的widget
    node->uiTitle = lineEdit_title;//指向标题控件
    node->uiDateYear = lineEdit_dateYear;//指向 日期“年” 这个控件
    node->uiDateMonth = lineEdit_dateMonth;//指向 日期“月” 这个控件
    node->uiDateDay = lineEdit_dateDay;//指向 日期“日” 这个控件
    node->uiTimeHour = lineEdit_timeHour;//指向 时间“小时” 这个控件
    node->uiTimeMinute = lineEdit_timeMinute;//指向 时间“分” 这个控件
    node->uiWeather = lineEdit_weather;//指向 天气 控件
    node->uiWeek = lineEdit_week;//指向 星期 控件
    node->uiMainBody = textEdit_mainBody;//指向 主体文本编辑 控件
    node->uiTimeBar = frame_3;//时间栏
    node->uiIsAvailable = 1;


    //添加内容变更 槽； uiNum不用宏，不想设置了，偷懒一下
    connect(lineEdit_dateYear, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 0);});
    connect(lineEdit_dateMonth, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 1);});
    connect(lineEdit_dateDay, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 2);});
    connect(lineEdit_timeHour, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 3);});
    connect(lineEdit_timeMinute, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 4);});
    connect(lineEdit_weather, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 5);});
    connect(lineEdit_week, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 6);});
    connect(lineEdit_title, &QLineEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 7);});
    connect(textEdit_mainBody, &QTextEdit::textChanged, this, [=](){funEdit_contentChangedAction(node->uniqueNum, 8);});

}

void MainWindow::funEdit_contentChangedAction(int num, int uiNum)
{
    TabLink *prob = this->link_method.fun_getTabByUniqueNum(this->link_head, num);
    int index = prob->fun_getIndex(this->link_head);
    //qDebug()<<"index "<<index<< " uiNum:"<<uiNum << " prob->isSave"<< prob->isSaved << "totalNum"<< this->link_method.fun_getTotalNum(this->link_head)
      //     <<" prob->isChangeCnt"<< prob->isChangeCnt;

    if(index < 0 || index >= this->link_method.fun_getTotalNum(this->link_head))
    {
        //超过范围
         qDebug()<<"out off range index"<<index;
        return;
    }

    if((prob->isChangeCnt <= 6 && prob->isNew == 1) ||
        (prob->isChangeCnt <= 6 && prob->isNew == 0))//新建0-6；打开0-5或者0-6（取决于有没有保存天气情况）,还是不严谨，以后再完善了
    {

        //prob->isSaved == 1;
        prob->isChangeCnt++;
        return;
    }




    //if( prob->isSaved == 1)
    {
        prob->isSaved = 0;//标记有未保存内容
        if(uiNum == 7)
        {
            prob->tabName.clear();
            prob->tabName.append(prob->uiTitle->text());

        }

        if(uiNum == 7 || uiNum == 1 || uiNum == 2)
        {
           // prob->pathExit = 1;//名字已更改
        }

        QString tmp(prob->tabName);
        tmp.append("*");

        this->ui_tabWidget->setTabText(index, tmp);
    }


}


//根据tab链表节点来更新内容
void MainWindow::funEdit_tabUpdateAll(TabLink *head)
{
    int i;
    for(i = 0; i < this->link_method.fun_getTotalNum(this->link_head); i++)
    {
        TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
        this->funEdit_tabUpdate(prob);
    }

}

//根据tab链表节点来更新内容
void MainWindow::funEdit_tabUpdate(TabLink *node)
{
//问题：如何抱证node指向的UI控件存在可用？如果ui控件不可用，程序会崩溃。
   if(node->uiIsAvailable == 0)
    {
        qDebug()<<"funEdit_tabUpdate error: uiIsAvailable = 0";
        return ;
    }

    QString qstr;
    node->uiTitle->setText(node->title);
    qstr.clear();
    if(node->year != -1) qstr.sprintf("%d", node->year);//注：-1代表无内容
    node->uiDateYear->setText(qstr);
    qstr.clear();
    if(node->month != -1)qstr.sprintf("%d", node->month);
    node->uiDateMonth->setText(qstr);
    qstr.clear();
    if(node->day != -1)qstr.sprintf("%d", node->day);
    node->uiDateDay->setText(qstr);
    qstr.clear();
    if(node->hour != -1)qstr.sprintf("%d", node->hour);
    node->uiTimeHour->setText(qstr);
    qstr.clear();
    if(node->minute != -1)qstr.sprintf("%d", node->minute);
    node->uiTimeMinute->setText(qstr);
    node->uiWeather->setText(node->weather);
    qstr.clear();
    if(node->week != -1)qstr.append(this->varEdit_weekStr[node->week]);
    node->uiWeek->setText(qstr);
    node->uiMainBody->setText(node->mainBody);

    if(node->isDialy == false)//非日记文本，则隐藏 标题栏和时间栏
    {
        node->uiTitle->hide();
        node->uiTimeBar->hide();

    }

}


//同步某一节点和编辑控件输入内容
void MainWindow::funEdit_saveToLink(TabLink *node)
{
    bool flag;
    int tmp;
    tmp = node->uiDateYear->text().toInt(&flag, 10);
    if(flag == true)
    {
        node->year = tmp;
    }
    tmp = node->uiDateMonth->text().toInt(&flag, 10);
    if(flag == true)
    {
        node->month = tmp;
    }
    tmp = node->uiDateDay->text().toInt(&flag, 10);
    if(flag == true)
    {
        node->day = tmp;
    }
    tmp = node->uiTimeHour->text().toInt(&flag, 10);
    if(flag == true)
    {
        node->hour = tmp;
    }
    tmp = node->uiTimeMinute->text().toInt(&flag, 10);
    if(flag == true)
    {
        node->minute = tmp;
    }
    node->title.clear();
    node->title.append(node->uiTitle->text());
    node->weather.clear();
    node->weather.append(node->uiWeather->text());
    node->mainBody.clear();
    node->mainBody.append(node->uiMainBody->toPlainText());


}

//同步所有编辑控件输入的内容
void MainWindow::funEdit_saveToLinkAll(TabLink *node)
{
    int i;
    for(i = 0; i < this->link_method.fun_getTotalNum(this->link_head); i++)
    {
        TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
        this->funEdit_saveToLink(prob);
    }

}



//注：mode 模式包括：1--新建  2--打开;文件名包括  时间+标题+后缀名
void MainWindow::funEdit_tabAdd(int mode, QString dir, QString fileName)
{
   TabLink *node = this->link_method.fun_add(this->link_head);
   node->lastFileName.clear();
   node->lastFileName.append(dir);
   node->lastFileName.append("/");
   node->lastFileName.append(fileName);

    if(mode == EDIT_TAB_ADD_MODE_NEW)
    {
        QDate qdate;
        QTime qtime;
        node->fileName.clear();
        node->fileName.append(fileName);
        node->dir.clear();
        node->dir.append(dir);
        node->title.clear();
        node->type.clear();
        node->year = qdate.currentDate().year();
        node->month = qdate.currentDate().month();
        node->day = qdate.currentDate().day();
        node->week = qdate.currentDate().dayOfWeek();
        node->hour = qtime.currentTime().hour();
        node->minute = qtime.currentTime().minute();
        node->weather.clear();
        node->mainBody.clear();
        node->type.append(".txt");
        node->isDialy = true;//默认新建日记文本
        node->pathExit = 0;//0代表不村存在
        node->tabName.clear();
        node->tabName.append(fileName);
        node->isNew = 1;

    }
    else if(mode == EDIT_TAB_ADD_MODE_OPEN)//普通方式打开文件
    {
        QString fullName;
        fullName.append(dir);
        fullName.append("/");
        fullName.append(fileName);
        QFile qfile(fullName);
        if(qfile.open(QIODevice::ReadWrite | QIODevice::Text) == false)
        {
            QMessageBox::warning(this, tr("警告"),tr("文件打开失败"));
            return;
        }
        node->mainBody.clear();
        QByteArray t = qfile.readAll();
        node->mainBody.append(QString(t));
        qfile.close();

        node->fileName.clear();
        node->fileName.append(fileName);
        node->dir.clear();
        node->dir.append(dir);
        node->title.clear();
        node->type.clear();
        node->year = -1;
        node->month = -1;
        node->day = -1;
        node->week = -1;//代表无
        node->hour = -1;
        node->minute = -1;
        node->weather.clear();
        node->isDialy = false;
        node->pathExit = 1;//存在且不需要替换

        node->tabName.clear();
        node->tabName.append(fileName);
        node->isNew = 0;

    }
    else if(mode == EDIT_TAB_ADD_MODE_OPEN_AS_DIALY)//分析文件名方式打开文件，默认文日记文档
    {
        int year, month, day;
        QString title, type;
        if(0 != analyseFileName(fileName, &year, &month, &day, &title, &type ))
        {
            //解析失败 转为普通方式打开
            funEdit_tabAdd(EDIT_TAB_ADD_MODE_OPEN, dir, fileName);
            return;
        }

        QDate qdate(year, month, day);
        node->year = year;
        node->month = month;
        node->day = day;
        node->title.clear();
        node->title.append(title);
        node->type.clear();
        node->type.append(type);
        //普通赋值      
        QString fullName;
        fullName.append(dir);
        fullName.append("/");
        fullName.append(fileName);
        QFile qfile(fullName);
        if(qfile.open(QIODevice::ReadWrite) == false)
        {
            QMessageBox::warning(this, tr("警告"),tr("文件打开失败"));
            return;
        }
        node->mainBody.clear();
        QByteArray t = qfile.readAll();
        QString content;
        content.clear();
        content.append(t);
        //对文本内容进行分析：这里约定文件格式;
        /*  ##start##
         *  标题
         *  作者：<用户名>
         *  日期：<年月日>  天气：<天气>[4个字符长度]星期<一...>
         *  ##end##
         */
        //主要获取天气内容
        node->weather.clear();

        int posStart = content.indexOf("##start##");//可以用宏表示
        int posEnd = content.indexOf("##end##");
        if(posStart != -1 || posEnd != -1 && posEnd > posStart)//头文件信息存在
        {

             QString str, tmp;
             int pos, pos2;
             int cnt = 0;
             str.append(content.mid(posStart, posEnd +6));//mid函数第一个参数是位置，第二个参数是个数，这里可能理解错了
            //qDebug()<<"===========>"<<str;

             while( cnt < 10)
             {
                 pos = str.indexOf("\n");

                 if(pos == -1)break;
                 tmp.clear();
                 tmp.append(str.mid(pos + 1, str.length()));
                 str.clear();
                 str.append(tmp);
                 cnt++;
                 if(cnt == 3)
                 {
                    pos = str.indexOf("天气：");
                    pos2 = str.indexOf("星期");
                    tmp.clear();
                    tmp.append(str.mid(pos, pos2));
                    pos2 = tmp.indexOf(" ");
                    tmp.clear();
                    tmp.append(str.mid(pos + 3, pos2 - 3));//需要测试调整

                    node->weather.append(tmp);
                 }

             }


             tmp.clear();
             tmp.append(content.mid(posEnd + 9, content.length()));
             content.clear();
             content.append(tmp);
        }


        node->mainBody.append(content);
        qfile.close();

        node->fileName.clear();
        node->fileName.append(fileName);
        node->dir.clear();
        node->dir.append(dir);
        node->week = qdate.dayOfWeek();
        node->hour = -1;
        node->minute = -1;//代表无
        node->isDialy = true;
        node->pathExit = 1;//保存时不再进入路径选取对话框
        node->tabName.clear();
        node->tabName.append(title);
        node->isNew = 0;
    }

    this->funEdit_tabWidgetCreate(node);
    this->funEdit_tabUpdate(node);
    this->ui_tabWidget->addTab(node->uiTab, node->tabName);
    this->ui_tabWidget->setCurrentIndex(node->fun_getIndex(this->link_head));
    //this->ui_tabWidget->setTabText(node->fun_getIndex(), "long");

}

//注:mode 模式包括：删除当前、根据索引号来删除、删除全部、删除其他
//如果不是根据索引号来删除，那么参数index不使用
void MainWindow::funEdit_tabDel(int mode, int index)
{
    //范围检查
    if(this->link_method.fun_getTotalNum(this->link_head) <= 0)
    {
        //没有节点
        return ;
    }
    QString tmp("是否保存");
    if(mode == EDIT_TAB_DEL_MODE_CURRENT)
    {
        int currentIndex = this->ui_tabWidget->currentIndex();
        //如果有未保存，请先保存
        TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, currentIndex);
        tmp.append(" “");
        tmp.append(prob->title);
        tmp.append("” ?");

        //qDebug()<<"==>prob->isSaved"<<prob->isSaved;
        if(prob->isSaved == 0)
        {
            QMessageBox::StandardButton  result=QMessageBox::question(this, "保存", tmp,
                              QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                              QMessageBox::NoButton); //缺省是no button
            if(result == QMessageBox::Yes)
            {
                if(-1 == this->funEdit_fileSave(currentIndex))
                {
                    return ;
                }

            }
            else if(result == QMessageBox::No)
            {
                ;
            }
            else
            {
                return;
            }

        }

        this->ui_tabWidget->removeTab(currentIndex);
        this->link_method.fun_del(this->link_head, currentIndex);
       // this->link_method.fun_showInfo(this->link_head);

    }
    else if(mode == EDIT_TAB_DEL_MODE_BYINDEX)
    {
        this->ui_tabWidget->removeTab(index);
        this->link_method.fun_del(this->link_head, index);
       // this->link_method.fun_showInfo(this->link_head);

    }
    else if(mode == EDIT_TAB_DEL_MODE_ALL)
    {
        int i;
        for(i = this->link_method.fun_getTotalNum(this->link_head) - 1; i >= 0; i--)
        {
            //如果有未保存，请先保存
            TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
            tmp.append(" “");
            tmp.append(prob->title);
            tmp.append("” ?");
            if(prob->isSaved == 0)
            {
                QMessageBox::StandardButton  result=QMessageBox::question(this, "保存", tmp,
                                  QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                                  QMessageBox::NoButton); //缺省是no button
                if(result == QMessageBox::Yes)
                {
                    if(-1 == this->funEdit_fileSave(i))
                    {
                        return ;
                    }

                }
                else if(result == QMessageBox::No)
                {
                    ;
                }
                else
                {
                    return;
                }
            }

            this->ui_tabWidget->removeTab(i);
            this->link_method.fun_del(this->link_head, i);
            //this->link_method.fun_showInfo(this->link_head);
        }
    }
    else if(mode == EDIT_TAB_DEL_MODE_OTHER)
    {
        int currentIndex;
        int i;
        for(i = this->link_method.fun_getTotalNum(this->link_head) - 1; i >= 0; i--)
        {
            currentIndex = this->ui_tabWidget->currentIndex();
            if(i != currentIndex)
            {
                //如果有未保存，请先保存
                TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
                tmp.append(" “");
                tmp.append(prob->title);
                tmp.append("” ?");
                if(prob->isSaved == 0)
                {
                    QMessageBox::StandardButton  result=QMessageBox::question(this, "保存", tmp,
                                      QMessageBox::Yes|QMessageBox::No |QMessageBox::Cancel,
                                      QMessageBox::NoButton); //缺省是no button
                    if(result == QMessageBox::Yes)
                    {
                        if(-1 == this->funEdit_fileSave(i))
                        {
                            return ;
                        }

                    }
                    else if(result == QMessageBox::No)
                    {
                        ;
                    }
                    else
                    {
                        return;
                    }
                }

                this->ui_tabWidget->removeTab(i);
                this->link_method.fun_del(this->link_head, i);
                // this->link_method.fun_showInfo(this->link_head);
            }
        }
    }

}

//文件打开，其中fileName没有路径
//打开模式
void MainWindow::funEdit_fileOpen(int mode, QString dir, QString fileName)
{
    //判断此文件是否已经打开，即dir 和fileName 都相等（链表提供方法）
    TabLink *prob = this->link_method.fun_fileExit(this->link_head, dir, fileName);
    if(NULL != prob)
    {

        this->ui_tabWidget->setCurrentIndex(prob->fun_getIndex(this->link_head));
        //qDebug()<<"funEditAction_new tip: file is open now!";
        //qDebug()<<"dir "<<dir;
        return ;
    }



    //可选：设置只打开.txt文件
    if(mode == EDIT_FILE_OPEN_GENERAL)
    {
        this->funEdit_tabAdd( EDIT_TAB_ADD_MODE_OPEN, dir, fileName);
    }
    else if(mode == EDIT_FILE_OPEN_ANALYSE)
    {
        this->funEdit_tabAdd( EDIT_TAB_ADD_MODE_OPEN_AS_DIALY, dir, fileName);
    }
}


//文件保存

int MainWindow::funEdit_fileSave(int tabIndex)
{

    TabLink *node = this->link_method.fun_getTabByIndex(this->link_head, tabIndex);
    this->funEdit_saveToLink(node);//数据同步到链表节点

    if(node->title.isEmpty() == true && node->isDialy == true)
    {
         QMessageBox::warning(this, tr("提示"),tr("请输入标题再保存！"));
         return -1;
    }


    QString fullName;

    QString adviceName, tmp;
    adviceName.append(this->var_fileScanDir).append("/");
    if(node->isDialy == true)
    {
        tmp.sprintf("%d-%d-%d-", node->year, node->month, node->day);
        tmp.append(node->title);
        tmp.append(node->type);

        adviceName.append(tmp);
        node->fileName.clear();
        node->fileName.append(tmp);
    }
    else
    {
        adviceName.append(node->fileName);
    }

    if(node->pathExit == 0)
    {
        fullName = QFileDialog::getSaveFileName(this,tr("保存"),adviceName ,tr("Text Files(*)"));
        if(fullName.isEmpty() == true)
        {
            //无保存名，保存失败
            return -1;
        }


        //##################################################################覆盖保存注意
        int pos;
        QString str(fullName);
        QString tmp1;
        QString fileName;
        while( str.indexOf("/")!= -1)
        {
            pos =  str.indexOf("/");
            tmp1.clear();
            tmp1.append(str.mid(pos + 1, str.length() - 1));
            str.clear();
            str.append(tmp1);
        }
        fileName.append(str);

        qDebug()<<"fileName "<<fileName<<"  node->fileName "<<tmp << "dir"<< node->dir;

        TabLink *prob = this->link_method.fun_fileExit(this->link_head, node->dir, fileName);//你要覆盖的文件已经打开了
        if(prob != NULL && fileName.compare(tmp) != 0)
        {
             QMessageBox::warning(this, tr("提示"),tr("你要保存的文件已再其他窗口打开！"));
             return -1;
        }

        if(fileName.compare(tmp) != 0 && isDialyName(fileName) == false)
        {
            QMessageBox::warning(this, tr("提示"),tr("你已保存为普通文件（非格式2020-10-1-name.txt）"));
            node->fileName.clear();
            node->fileName.append(fileName);
            node->tabName.clear();
            node->tabName.append(fileName);
            node->isDialy = false;
            this->funEdit_tabUpdate(node);

        }
        //==========================================================================

        node->pathExit = 1;
    }
    else if(node->pathExit == 1)
    {
        fullName.clear();
        fullName.append(adviceName);
    }

   // qDebug()<<"node->lastFileName: "<< node->lastFileName<<"  fullName:"<<fullName;

    //打开上一次的保存名
    QFile qfile(node->lastFileName);
    if(qfile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate) == false)
    {
        QMessageBox::warning(this, tr("警告"),tr("文件打开失败"));
        return -1;
    }

    if(fullName.compare(node->lastFileName) != 0)
    {
        //记录重命名
        cloud_file_int->fileRecord_rename( node->lastFileName, fullName);

        node->lastFileName.clear();
        node->lastFileName.append(fullName);
        qfile.rename(fullName);

    }

    QString content;
    /*  ##start##
     *  标题
     *  作者：<用户名>
     *  日期：<年月日>  天气：<天气>[4个字符长度]星期<一...>
     *  ##end##
     */
    QString qstrTmp;
    if(node->isDialy == true)
    {
        content.append("\n##start##\n");
        content.append(node->title);
        content.append("\n作者：");
        content.append(this->varUser_name);
        content.append("\n");
        content.append("日期：");
        qstrTmp.sprintf("%d年%d月%d日", node->year, node->month, node->day);
        content.append(qstrTmp);
        content.append("    天气：");
        content.append(node->weather);
        content.append("    星期");
        content.append(this->varEdit_weekStr[node->week]);
        content.append("\n##end##\n");

    }
    content.append(node->mainBody);
    qfile.write(content.toUtf8());

    qfile.close();

    node->isSaved = 1;
    this->ui_tabWidget->setTabText(tabIndex, node->tabName);
    //更新 文件浏览页面
    funFileScan_ModeChange(this->var_fileScanMode);


    //记录保存(新建或覆盖动作)
    QString empty;
    qDebug()<<"================>funEdit_fileSave: isNew="<<node->isNew<<" isDialy="<<node->isDialy<<" fileName="<<node->fileName;
    if(node->isDialy == true)
    {
        if(node->isNew == 1)
        {
            cloud_file_int->fileRecord_addFile(node->fileName, empty);
        }
        else
        {
            cloud_file_int->fileRecord_modifyFile(node->fileName.toUtf8(), content);
        }
    }

    node->isNew = 0;
    return 0;
}





//新建、打开、保存、另存为 动作对应的槽函数
void MainWindow::funEditAction_new()
{
    //qDebug()<<"funEditAction_new is comming";

    this->funEdit_tabAdd(EDIT_TAB_ADD_MODE_NEW, this->var_fileScanDir, "new");


}
void MainWindow::funEditAction_open()
{

    QString path=QFileDialog::getOpenFileName(this,tr("打开"),this->var_fileScanDir,tr("Text Files(*)"));

    //拆解为目录和文件名
    int pos, posAll;
    QString str(path);
    QString tmp;
    QString dir;
    QString fileName;

    posAll = 0;
    while( str.indexOf("/")!= -1)
    {
        pos =  str.indexOf("/");
        posAll +=pos + 1;
        tmp.clear();
        tmp.append(str.mid(pos + 1, str.length() - 1));
        str.clear();
        str.append(tmp);
    }
    fileName.append(str);
    str.clear();
    str.append(path);
    dir.append(str.mid(0, posAll - 1));

    //qDebug()<<"dir"<<dir<<" fileName"<<fileName;

    //判断此文件是否已经打开，即dir 和fileName 都相等（链表提供方法）
    /*TabLink *prob = this->link_method.fun_fileExit(this->link_head, dir, fileName);
    if(NULL != prob)
    {

        this->ui_tabWidget->setCurrentIndex(prob->fun_getIndex());
        qDebug()<<"funEditAction_new tip: file is open now!";
        qDebug()<<"dir "<<dir;
        return ;
    }*/

    //通过文件名来判断是否为 日记文本，例如2020-10-1-dfa.txt
    if(true == isDialyName(fileName))
    {
         this->funEdit_fileOpen(EDIT_FILE_OPEN_ANALYSE, dir, fileName);
    }
    else
    {
        this->funEdit_fileOpen(EDIT_FILE_OPEN_GENERAL, dir, fileName);
    }

}

void MainWindow::funEditAction_save()//保存当前页面
{


    int currentIndex = this->ui_tabWidget->currentIndex();

    if(currentIndex == -1)
    {
        //无窗口
        return;
    }
    this->funEdit_fileSave(currentIndex);

}

void MainWindow::funEditAction_saveAll()//保存所有“未保存”且“有确定路径文件名”的对象
{
    int i;
    for(i = 0; i < this->link_method.fun_getTotalNum(this->link_head); i++)
    {
        TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
        //if(prob->pathExit == 1 && prob->isSaved == 0)
        if( prob->isSaved == 0)
        {
            this->funEdit_saveToLink(prob);
            this->funEdit_fileSave(i);
        }
    }
}

void MainWindow::funEditAction_saveAs()
{
    int currentIndex = this->ui_tabWidget->currentIndex();

    if(currentIndex == -1)
    {
        //无窗口
        return;
    }
    TabLink *node = this->link_method.fun_getTabByIndex(this->link_head, currentIndex);
    this->funEdit_saveToLink(node);//数据同步到链表节点


    QString fullName = QFileDialog::getSaveFileName(this,tr("保存"),this->var_fileScanDir ,tr("Text Files(*)"));
    if(fullName.isEmpty() == true)
    {
        //无保存名，保存失败
        return;
    }


    QFile qfile(fullName);
    if(qfile.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        QMessageBox::warning(this, tr("警告"),tr("文件打开失败"));
        return;
    }

    qfile.write(node->mainBody.toUtf8());
    qfile.close();

}
void MainWindow::funEditAction_close()
{
    this->funEdit_tabDel(EDIT_TAB_DEL_MODE_CURRENT, -1);

}
void MainWindow::funEditAction_closeAll()
{

    this->funEdit_tabDel(EDIT_TAB_DEL_MODE_ALL, -1);
}

void MainWindow::funEditAction_closeOther()
{
    this->funEdit_tabDel(EDIT_TAB_DEL_MODE_OTHER, -1);
}
//==========================================================



//###########################################################其他
//测试按钮
void MainWindow::userSetOk()
{
    QString para;
    QString strTmp;
    //用户名设置并保存
    para.clear();
    para.append(this->userNameEdit->text());
    this->varUser_name.clear();
    this->varUser_name.append(para);
    ui->pushButton->setText(para.mid(0, 1));
    strTmp.clear();
    strTmp.append("用户名：");
    strTmp.append(para);
    this->varUser_name.clear();
    this->varUser_name.append(para);
    ui->label->setText(strTmp);
    this->saveCfgFile(this->var_userDefaultDir, "dialy.cfg", tr("USER_NAME"), para);//存入文件

    //工作路径设置并保存（注：工作路径 和用户路径不同  一个是wkpath，另一个是wkpath/user/<user_name>）
    para.clear();
    para.append(this->workDirEdit->text());
    this->app_work_path.clear();
    this->app_work_path.append(para);

    //更新
    cloud_file_int->file_opt_int->setCurrentUserFloder(app_work_path, cloud_file_cfg.user_name);
    fileSacnUpdate(FILE_SCAN_DIALY_MODE, cloud_file_int->file_opt_int->current_user_path);
    //qDebug()<<"=================>current_user_path="<<cloud_file_int->file_opt_int->current_user_path;
    this->saveCfgFile(this->var_userDefaultDir, "dialy.cfg", tr("WORK_PATH"), para);//存入文件

    this->userDlg->close();
    delete this->userDlg;
}
void MainWindow::userSetCancel()
{
    this->userDlg->close();
    delete this->userDlg;

}

//用户设置按钮
void MainWindow::on_pushButton_clicked()
{
  /*  static int cnt = 0;

    ui->pushButton->setStyleSheet(this->var_sheetColor[cnt % 8]);
    char tmp[1];
    tmp[0] = ('A' + cnt % ('z' - 'a'));

    ui->pushButton->setText(tmp);
    QString str("用户名：");
    str.append(USER_INFO_USER_DEFAULT_NAME);
    ui->label->setText(str);
    cnt++;*/


    this->funUser_setPara("用户设置");

}


//用户设置界面
void MainWindow::funUser_setPara(QString prompt)
{
    userDlg = new QDialog(this);
    QVBoxLayout *layout= new QVBoxLayout(userDlg);
    //用户名设置
    userDlg->setWindowTitle(prompt);
    QLabel *promptLabel = new QLabel(userDlg);
    promptLabel->setText("请输入你的姓名");
    userNameEdit = new QLineEdit(userDlg);
    layout->addWidget(promptLabel);
    layout->addWidget(userNameEdit);
     userNameEdit->setText(this->varUser_name);
    //新增设置保存路径并保存到配置文件
    QLabel *promptLabel_2 = new QLabel(userDlg);
    promptLabel_2->setText("请输入工作路径");
     workDirEdit = new QLineEdit(userDlg);
    layout->addWidget(promptLabel_2);
    layout->addWidget(workDirEdit);
    workDirEdit->setText(this->app_work_path);
    //确定按钮
    QPushButton *btnOk = new QPushButton(userDlg);
    btnOk->setText("确定");
    QPushButton *btnCancel = new QPushButton(userDlg);
    btnCancel->setText("取消");

    layout->addWidget(btnOk);
    layout->addWidget(btnCancel);
    //槽
    userDlg->show();
    connect(btnOk, SIGNAL(clicked()), this, SLOT(userSetOk()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(userSetCancel()));

}




//系统时间显示
void MainWindow::timerUpdate()
{
   QString dateTime;
   dateTime.append("系统时间:");
   dateTime.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss dddd"));
   ui->label_systemTime->setText(dateTime);

   //计数
   if(time_updata_count < 0 || time_updata_count > 10000000)
   {
       time_updata_count = 0;
   }
   time_updata_count++;

   //云端服务的超时处理
   cloud_server_time_out_cnt++;
   if(cloud_server_time_out_cnt == cloud_server_time_out_num && cloud_server_time_out_en == 1)
   {
        cloudFile_timeoutPro();
   }
   if(cloud_server_time_count_disp_en == 1)
   {
        cloudFile_timeCountDisp();
   }

   //自动同步处理
   //qDebug()<<"timerUpdate: cloud_file_auto_sync_en="<<cloud_file_auto_sync_en<<" cloud_file_auto_sync_para"<<cloud_file_auto_sync_para
   //         <<"  time_updata_count="<<time_updata_count;
   if(1 == cloud_file_auto_sync_en && cloud_file_auto_sync_para > 0 && (time_updata_count % (cloud_file_auto_sync_para * 60)) == 0)
   {
        cloudFile_oneShootSync();
   }

}

//快捷键方法：[ctr+s]--保存 [ctr+n]--新建 [ctr+f]--查找
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_S))
    {
        this->funEditAction_save();
    }
    else if((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_N))
    {
        this->funEditAction_new();

    }
    else if((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_F))
    {
        this->findDlg->show();
    }
    else if((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_W))
    {
        this->funEditAction_close();
    }
    else if((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_V))//粘贴快捷键
    {
//        //刷新内容，消除粘贴的格式
//        TabLink *node = this->link_method.fun_getTabByIndex(this->link_head, this->ui_tabWidget->currentIndex());
//        if(NULL != node)
//        {
//             QString t = node->uiMainBody->toPlainText();
//             node->uiMainBody->clear();
//             node->uiMainBody->setText(t);
//        }
//        qDebug()<<"ctr+v is OK";
    }



}


void MainWindow::showFindText()
{
    //qDebug()<<"OK";
    QString str = this->findLineEdit->text();

    int currentIndex = this->ui_tabWidget->currentIndex();
    TabLink *node = this->link_method.fun_getTabByIndex(this->link_head, currentIndex);
    if (! node->uiMainBody->find(str))
    {
        if(! node->uiMainBody->find(str, QTextDocument::FindBackward))//往回寻找
            {
            QMessageBox::warning(this, tr("查找"),
                     tr("找不到%1").arg(str));
        }

    }

}



//###########################################################用户信息
void MainWindow::funUser_init()
{
    this->varUser_name.clear();
    this->varUser_name.append(USER_INFO_USER_DEFAULT_NAME);

    QString para;
    QString tmp;

    //取出当前工作目录：必须要有起始目录，比如家目录  QDir::homePath()；默认路径用于存放配置文件dialy.cfg

    QDir qdir;
    QFile qfile;
    if(true == qdir.cd(QDir::homePath()))//如果家路径不在，则换成本应用所在路径
    {
           this->var_userDefaultDir.clear();
           this->var_userDefaultDir.append(QDir::homePath());
    }
    else if(false == qdir.cd(QDir::currentPath()))
    {
        this->var_userDefaultDir.clear();
        this->var_userDefaultDir.append(QDir::currentPath());

    }
    else
    {
        this->var_userDefaultDir.clear();
        this->var_userDefaultDir.append("/");//以后完善
    }

    tmp.clear();
    tmp.append(this->var_userDefaultDir);
    tmp.append("/");
    tmp.append("dialy.cfg");
    qfile.setFileName(tmp);
    if(qfile.exists() == true)
    {
        this->getParaFromCfgFile(this->var_userDefaultDir, "dialy.cfg", "WORK_PATH", &para);
        if(para.isEmpty() == false)
        {
            this->app_work_path.clear();
            this->app_work_path.append(para);//设置工作路径
        }
        else
        {
            this->app_work_path.clear();
            this->app_work_path.append(this->var_userDefaultDir);//设置工作路径
        }

    }
    else//不存在就创建，也代表是第一次使用本软件   home 目录创建 dialy.cfg
    {
        if(false == qfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qDebug()<<"funUser_init:  qfile.open is error";
        }
        qfile.close();
        //进入用户设置
        this->app_work_path.clear();
        this->app_work_path.append(this->var_userDefaultDir);//设置工作路径

        this->funUser_setPara("温馨提示：第一次使用请设置你自己喜欢的工作目录");
       //创建完后，再次进入初始化
        //this->funUser_init();
        //return ;
    }
    this->var_fileScanLabelDir->setText("默认路径");//设置工作目录 提示标签
    this->var_fileScanLineEditDir->setText(this->app_work_path);//填充工作路径到编辑框
    //fileSacnUpdate(FILE_SCAN_DIALY_MODE, para);



    //取出参数用户名   
    this->getParaFromCfgFile(this->var_userDefaultDir, "dialy.cfg", "USER_NAME", &para);
    qDebug()<< "para "<<para;
    if(para.isEmpty() == false)
    {
        this->varUser_name.clear();
        this->varUser_name.append(para);

        ui->pushButton->setText(para.mid(0, 1));
        QString str("用户名：");
        str.append(para);
        ui->label->setText(str);
    }



    //其他初始化内容
    this->flag_leftHide = 0;
    ui->action_showLeft->setText("隐藏左边");

}

//存入配置文件
void MainWindow::saveCfgFile_byName(QString name, QString entry, QString para)
{
    QFile qfile(name);

    qfile.open(QIODevice::ReadOnly | QIODevice::Text );
    QByteArray t = qfile.readAll();
    QString content(t);
    qfile.close();


    QString entryTmp(entry);
    entryTmp.append("=");
    int pos = content.indexOf(entryTmp);//注意 参数名应该再加上“=”

    //qDebug()<<"pos"<<pos<< " content"<< content;
    if(pos != -1)
    {
        QString str(content.mid(pos , content.length()));

        int pos2 = str.indexOf("\n");
        if(pos2 == -1)
        {
            pos2 = str.length();
        }


        //qDebug()<<"str"<<str;
        QString section1, section2;
        if( pos  > 0)
        {
            section1.append(content.mid(0, pos ));
        }
         if( content.length() > (pos + pos2 ))
        {
            section2.append(content.mid(pos + pos2 , content.length() - pos - pos2 + 1));
        }


        //qDebug()<<"sec1"<<section1<<"  sec2"<<section2;

        QString output(entry);
        output.append("=");
        output.append(para);
        //output.append("\n");

        content.clear();
        content.append(section1);
        content.append(output);
        content.append(section2);
    }
    else
    {
        QString output("\n");
        output.append(entry);
        output.append("=");
        output.append(para);


        content.append(output);
    }

    qfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);//非追加
    qfile.write(content.toUtf8());

    qfile.close();

}

//存入配置文件  dialy.cfg
void MainWindow::saveCfgFile(QString dir, QString fileName, QString entry, QString para)
{
    QString fullName(dir);
    fullName.append("/");
    fullName.append(fileName);

    saveCfgFile_byName(fullName, entry, para);

}

//取出配置文件参数
int MainWindow::getParaFromCfgFile_byName(QString name, QString entry, QString *para)
{
    QFile qfile(name);

    //qDebug()<<"fullName: "<<fullName;
    qfile.open(QIODevice::ReadOnly | QIODevice::Text );
    QByteArray t = qfile.readAll();
    QString content(t);
    qfile.close();

    //分析
    QString entryTmp(entry);
    entryTmp.append("=");
    int pos = content.indexOf(entryTmp);//注意 参数名应该再加上“=”
    int pos2;
    QString str;
    QString tmp;
    int pos3;
    int pos4;
    int errorCode = 0;

    if(pos == -1){errorCode=1; goto not_found_error;}
    //qDebug()<<"content: "<<content;
    str.append(content.mid(pos , content.length() - pos + 1));//之前理解错mid函数了，真确理解是mid 第一个参数是位置，第二个参数是个数
    //qDebug()<<"str1: "<<str;
    pos2 = str.indexOf("\n");
    if(pos2 != -1)
    {
        tmp.append(str.mid(0, pos2));
        str.clear();
        str.append(tmp);

    }


    pos3 = str.indexOf("=");
    if(pos3 == -1){errorCode=3; goto not_found_error;}

    pos4 = str.indexOf(" ");

    if(pos4 == -1)
    {
        pos4 = str.indexOf("\n");
        if(pos4 == -1)
        {
            if( str.length() == 0){errorCode=4; goto not_found_error;}
            pos4 = str.length() - 1;
        }

    }
    if(pos4 < pos3){errorCode=5; goto not_found_error;}
    tmp.clear();
    //qDebug()<<"str2: "<<str;

    tmp.append(str.mid(pos3 + 1, pos4 - pos3 ));
    //qDebug()<<"pos4："<<pos4<< "pos3："<< pos3<< "tmp："<< tmp;
    if(tmp.isEmpty() == false)
    {
        para->clear();
        para->append(tmp);
    }
    return 0;

not_found_error:
    qDebug()<<"getParaFromCfgFile_byName error: not found"<< " errorCode"<< errorCode<< " file name:"<<name<<" entry="<<entry;
    para->clear();
    return errorCode;


}

//取出配置文件参数  dialy.cfg
void MainWindow::getParaFromCfgFile(QString dir, QString fileName, QString entry, QString *para)
{
    //qDebug()<<"homeDir: "<<QDir::homePath();
    QString fullName(dir);   //注意：要先设置工作路径，配置文件才有存放的地方
    fullName.append("/");
    fullName.append(fileName);

    getParaFromCfgFile_byName(fullName, entry, para);
}
//注：goto 语句后面又出现了新定义的变量，会出错


//隐藏左边控件
void MainWindow::on_pushButton_hideLeft_clicked()
{
    ui->frame_left->hide();
    this->flag_leftHide = 1;
    ui->action_showLeft->setText("展开左边");
}
//展开左边控件
void MainWindow::on_action_showLeft_triggered()
{
    if(this->flag_leftHide == 1)
    {
        this->flag_leftHide = 0;
        ui->frame_left->show();

        ui->action_showLeft->setText("隐藏左边");
    }
    else if(this->flag_leftHide == 0)
    {
        this->flag_leftHide = 1;
        ui->frame_left->hide();

        ui->action_showLeft->setText("展开左边");

    }

}

//主编辑区的字体大小设置
void MainWindow::funOther_setFontSize(int mode)//自增或者自减  1--自增  2--自减
{
    if(mode == 1)
    {
        if(this->varFont_size <= 19)
        {
            this->varFont_size++;
        }

    }
    else if(mode == 2)
    {
        if(this->varFont_size >= 9)
        {
            this->varFont_size--;
        }

    }


    int i;
    int num = this->link_method.fun_getTotalNum(this->link_head);
    for(i = 0; i < num; i++)
    {

        TabLink *prob = this->link_method.fun_getTabByIndex(this->link_head, i);
        if(NULL != prob)
        {
            QFont font;
            font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
            font.setPointSize(this->varFont_size);
            prob->uiMainBody->setFont(font);
        }

    }

}

//#####################################################################云端同步功能
///////////////////////////////////////////////////////（非类方法）
int cutStr(char *from, char *to, int start, int len)
{
    int i;
    for(i = 0; i < len; i ++)
    {
        to[i] = from[start + i];
    }
    to[i] = '\0';
    return 0;
}
int ipStrToChar(char *ipstr, unsigned char *ipchar)
{
    char tmp[8] = {0};
    int len = strlen(ipstr);
    int i;
    int pos[5] = {0};
    int cnt;
    cnt = 1;
    int sub;
    for(i = 0; i < len; i++)
    {
        if(ipstr[i] == '.')
        {
            pos[cnt] = i;
            cnt++;
            if(cnt >= 4)
            {
                break;
            }
        }

    }
    //qDebug()<<"==============>ipStrToChar "<<pos[0]<<" "<<pos[1]<<" "<<pos[2]<<" "<<pos[3];
    if(cnt != 4)
    {
        ipchar[0] = ipchar[1] = ipchar[2] = ipchar[3] = 0;
         //qDebug()<<"ipStrToChar: ip transform error 0";
        return -1;
    }
    pos[0] = -1;
    pos[4] = pos[3] + 4;

    for(i = 0; i < 4; i++)
    {
        sub = pos[i + 1] - pos[i] - 1;
        if(sub < 1 || sub > 3)
        {
            //qDebug()<<"ipStrToChar: ip transform error 1, sub="<<sub;
            return -1;
        }
        cutStr(ipstr, tmp, pos[i] + 1, sub);
        //qDebug()<<"==============>tmp="<<tmp;
        ipchar[i] = atoi(tmp);
        if(ipchar[i] == 0 && tmp[0] != '0')
        {
            //qDebug()<<"ipStrToChar: ip transform error 2\n";
            return -1;
        }

    }
    return 0;
}
int ipCharToStr(char *ipstr, unsigned char *ipchar)
{
    sprintf(ipstr, "%d.%d.%d.%d", (unsigned char)ipchar[0], (unsigned char)ipchar[1], (unsigned char)ipchar[2], (unsigned char)ipchar[3]);
    return 0;
}
///////////////////////////////////////////////////////////////初始化
#if 0
typedef struct strcut_CLOUD_FILE_CFGtmp{
    qint8 rem_passwd;  //记住密码（0代表不记住密码  1代表记住密码）
    unsigned char server_ip[4];  //服务器ip
    qint8 port;     //端口
    qint32 auto_sync_para;   //自动同步时间间隔参数（单位分钟）
    qint8 auto_sync_en;     //使能自动同步（0 代表非使能  1代表使能， 默认值 0）
    qint8 startup_sync_en;     //打开应用，就尝试连接服务器，并进行同步（0 否  1 是  默认值 0）
    char user_name[64];     //记住的用户名
    char user_passwd[64];    //记住的密码（已加密）
}strcut_CLOUD_FILE_CFG;
#endif

//初始化
extern int setMainWindowIntf(MainWindow *m);
//extern int serverReplyToMainWindow(int action_num, int reply_code, QString msg);
int MainWindow::cloudInit()
{
    QString user_name;
    user_name.clear();
    //其他
     ui->lineEdit_user_passwd->setEchoMode(QLineEdit::Password);   //设置密码输入的模式
     cfg_passwd_change_en = 1;

    //初始化 cloudFile 类接口
    cloud_file_int = new cloudFile(app_work_path);  //当前工作目录

    //获取上一次的用户名（存于  "dialy.cfg"）
    this->getParaFromCfgFile(this->var_userDefaultDir, "dialy.cfg", "LAST_USER_NAME", &user_name);
    if(user_name.isEmpty())
    {
        user_name.append("guest");
    }

    //设置用户目录
    cloud_file_int->file_opt_int->setCurrentUserFloder(app_work_path, user_name);//设置当前用户文件夹
    var_fileScanDir.clear();
    var_fileScanDir.append(cloud_file_int->file_opt_int->current_user_path);

    //取出配置文件的信息并执行
    cloudCfgFileGetInfo(cloud_file_int->file_opt_int->current_user_cfg_file, &cloud_file_cfg);
    scloudCfgInfoShow(cloud_file_cfg);
    cloudCfgExe(cloud_file_cfg);

    //日记浏览
    fileSacnUpdate(FILE_SCAN_DIALY_MODE, cloud_file_int->file_opt_int->current_user_path);

    //隐藏内容
    ui->widget_user_cont->hide();
    ui->widget_cfg_cont->hide();
    ui->label_user_prompt->hide();
    ui->label_cfg_prompt->hide();

    //密码是否有新的输入标记
    cfg_passwd_change_mark = 0;

    //槽绑定
    connect(ui->pushButton_user_user, SIGNAL(clicked()), this, SLOT(bthUserUserClicked()));
    connect(ui->pushButton_cfg_cfg, SIGNAL(clicked()), this, SLOT(bthCfgCfgClicked()));
    connect(ui->pushButton_debug, SIGNAL(clicked()), this, SLOT(mydebug()));
    //-1 cfg 更改未保存提示
    connect(ui->lineEdit_cfg_ip, SIGNAL(textChanged(const QString &)), this, SLOT(cfgCfgNotSavePrompt()));
    connect(ui->lineEdit_cfg_port, SIGNAL(textChanged(const QString &)), this, SLOT(cfgCfgNotSavePrompt()));
    connect(ui->lineEdit_cfg_autosync, SIGNAL(textChanged(const QString &)), this, SLOT(cfgCfgNotSavePrompt()));
    connect(ui->checkBox_cfg_autonsync, SIGNAL(toggled(bool)), this, SLOT(cfgCfgNotSavePrompt()));
    connect(ui->checkBox_cfg_power_on_sync, SIGNAL(toggled(bool)), this, SLOT(cfgCfgNotSavePrompt()));
    //-2 密码是否有新的输入标记  密码的改动要及时地存入类成员 cloud_file_cfg
    connect(ui->lineEdit_user_passwd, SIGNAL(textChanged(const QString &)), this, SLOT(cfgUserPasswdChanged()));
    //-3 配置保存、取消
    connect(ui->pushButton_cfg_save, SIGNAL(clicked(bool)), this, SLOT(bthCfgSaveClicked()));
    connect(ui->pushButton_cfg_cancel, SIGNAL(clicked(bool)), this, SLOT(bthCfgCancelClicked()));
    //-4 登录、注册、注销、同步、销毁
     connect(ui->pushButton_user_login, SIGNAL(clicked(bool)), this, SLOT(bthUserLoginClicked()));
     connect(ui->pushButton_user_register, SIGNAL(clicked(bool)), this, SLOT(bthUserRegisterClicked()));
     connect(ui->pushButton_user_logout, SIGNAL(clicked(bool)), this, SLOT(bthUserLogoutClicked()));
     connect(ui->pushButton_user_sync, SIGNAL(clicked(bool)), this, SLOT(bthUserSyncClicked()));
     connect(ui->pushButton_user_destroy, SIGNAL(clicked(bool)), this, SLOT(bthUserDestoryClicked()));
     //-5 本地创建、用户切换、连接测试
     connect(ui->pushButton_user_localCreate, SIGNAL(clicked(bool)), this, SLOT(bthUserlocalCreateClicked()));
     connect(ui->pushButton_user_switch, SIGNAL(clicked(bool)), this, SLOT(bthUserSwitchClicked()));
     connect(ui->pushButton_user_connectTest, SIGNAL(clicked(bool)), this, SLOT(bthUserConnectTestClicked()));



    //给cloudfile.cpp 提供本函数所处类的接口
    setMainWindowIntf(this);

    //关闭超时计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 0;

    //部分控件属性设置
     ui->label_user_prompt->setStyleSheet("color:red;");
     UserDestory_diag_int();
}

///////////////////////////////////////////////////////按键互动
//用户按钮
int MainWindow::bthUserUserClicked()
{
    static int a = 0;
    if(a == 0)
    {
        a = 1;
        ui->pushButton_user_user->setText("用户<<");
        ui->widget_user_cont->show();
    }
    else if(a == 1)
    {
        a = 0;
        ui->pushButton_user_user->setText("用户>>");
        ui->widget_user_cont->hide();
    }
    else
    {
        a = 0;
    }

}
//配置按钮
int MainWindow::bthCfgCfgClicked()
{
    static int a = 0;
    if(a == 0)
    {
        a = 1;
        ui->pushButton_cfg_cfg->setText("配置<<");
        ui->widget_cfg_cont->show();
    }
    else if(a == 1)
    {
        a = 0;
        ui->pushButton_cfg_cfg->setText("配置>>");
        ui->widget_cfg_cont->hide();
    }
    else
    {
        a = 0;
    }
    return 0;

}
//保存配置
int MainWindow::bthCfgSaveClicked()
{
    //strcut_CLOUD_FILE_CFG cloud_file_cfg;
    cloudCfgGetBaordCfgInfo(&cloud_file_cfg);
    // qDebug()<<"bthCfgSaveClicked:A user_passwd="<<cloud_file_cfg.user_passwd;
    cloudCfgExe(cloud_file_cfg);
    //qDebug()<<"bthCfgSaveClicked:B user_passwd="<<cloud_file_cfg.user_passwd;
    cloudCfgSaveToFile(cloud_file_int->file_opt_int->current_user_cfg_file, cloud_file_cfg);


    ui->label_cfg_prompt->hide();



    //本次用户名存入配置文件
    QString name(cloud_file_cfg.user_name);
    this->saveCfgFile(this->var_userDefaultDir, "dialy.cfg", tr("LAST_USER_NAME"), name);



    return 0;
}
//取消配置
int MainWindow::bthCfgCancelClicked()
{
    cloudCfgExe(cloud_file_cfg);
    ui->label_cfg_prompt->hide();

    return 0;
}

//本地创建
int MainWindow::bthUserlocalCreateClicked()
{
    QString user_name(ui->lineEdit_user_name->text());
    //初始化 cloudFile 类接口
    cloud_file_int = new cloudFile(app_work_path);  //当前工作目录
    //设置用户目录
    cloud_file_int->file_opt_int->userFloderInit(app_work_path, user_name);
    cloud_file_int->file_opt_int->setCurrentUserFloder(app_work_path, user_name);//设置当前用户文件夹
    var_fileScanDir.clear();
    var_fileScanDir.append(cloud_file_int->file_opt_int->current_user_path);
    //初始化配置
    cloud_file_cfg.rem_passwd = 0;
    cloud_file_cfg.server_ip[0] = 0;
    cloud_file_cfg.server_ip[1] = 0;
    cloud_file_cfg.server_ip[2] = 0;
    cloud_file_cfg.server_ip[3] = 0;
    cloud_file_cfg.port = 0;
    cloud_file_cfg.auto_sync_para = 0;
    cloud_file_cfg.auto_sync_en = 0;
    cloud_file_cfg.startup_sync_en = 0;
    strcpy(cloud_file_cfg.user_name, user_name.toLatin1().data());
    memset(cloud_file_cfg.user_passwd, '\0', strlen(cloud_file_cfg.user_passwd));

    //在面板上体现配置信息（执行配置）
    cloudCfgExe(cloud_file_cfg);

    //日记浏览
    fileSacnUpdate(FILE_SCAN_DIALY_MODE, cloud_file_int->file_opt_int->current_user_path);
}
//用户切换
int MainWindow::bthUserSwitchClicked()
{
    QString user_name(ui->lineEdit_user_name->text());
    cloud_file_int->file_opt_int->setCurrentUserFloder(app_work_path, user_name);
    var_fileScanDir.clear();
    var_fileScanDir.append(cloud_file_int->file_opt_int->current_user_path);
    this->fileSacnUpdate(FILE_SCAN_DIALY_MODE, var_fileScanDir);
}

/////////////////////////////////////////////云端服务
//连接测试  action_num = 1
int MainWindow::bthUserConnectTestClicked()
{
    QString msg("hello, I am dialy v4 user ");
    msg.append(cloud_file_int->file_opt_int->current_user_name);

    //提示语
    QString prompt("提示：连接测试中...");
    ui->label_user_prompt->setText(prompt);
    ui->label_user_prompt->show();

     //连接服务器
    char tmp[64] = {0};
    ipCharToStr(tmp, cloud_file_cfg.server_ip);
    QString ip(tmp);
    qDebug()<<"连接测试： ip="<<ip<< "  port="<<cloud_file_cfg.port;
    cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);


    //发送信息
    if(cloud_file_int->client_sock_mark == 1)
    {
        cloud_file_int->sayHello(msg);
    }

    //不开始开始超时检测，但是计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 1;
    cloud_server_time_count_str.clear();
    cloud_server_time_count_str.append(prompt);
    cloud_server_time_out_cnt = 0;


    return 0;
}
//登录 action_num = 2
int MainWindow::bthUserLoginClicked()
{
    //取得参数
    QString user_name(ui->lineEdit_user_name->text());
    QString user_passwd(cloud_file_cfg.user_passwd);

    //提示语
    QString prompt;
    prompt.sprintf("提示：用户%s登录中...", user_name.toLatin1().data());
    ui->label_user_prompt->setText(prompt);
    ui->label_user_prompt->show();

     //连接服务器
    char tmp[64] = {0};
    ipCharToStr(tmp, cloud_file_cfg.server_ip);
    QString ip(tmp);
    qDebug()<<"连接测试： ip="<<ip<< "  port="<<cloud_file_cfg.port;
    cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);

    //发送信息
    if(cloud_file_int->client_sock_mark == 1)
    {
        cloud_file_int->reqUserLogin(user_name, user_passwd);
    }
    else
    {
        ui->label_user_prompt->setText("未与服务器连接");
    }

    //不开始开始超时检测，但是计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 1;
    cloud_server_time_count_str.clear();
    cloud_server_time_count_str.append(prompt);
    cloud_server_time_out_cnt = 0;
}
//注册 action_num = 3
int MainWindow::bthUserRegisterClicked()
{
    //取得参数
    QString user_name(ui->lineEdit_user_name->text());
    QString user_passwd(cloud_file_cfg.user_passwd);

    //提示语
    QString prompt;
    prompt.sprintf("提示：用户%s注册中...", user_name.toLatin1().data());
    ui->label_user_prompt->setText(prompt);
    ui->label_user_prompt->show();

     //连接服务器
    char tmp[64] = {0};
    ipCharToStr(tmp, cloud_file_cfg.server_ip);
    QString ip(tmp);
    qDebug()<<"连接测试： ip="<<ip<< "  port="<<cloud_file_cfg.port;
    cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);

    //发送信息
    if(cloud_file_int->client_sock_mark == 1)
    {
        cloud_file_int->reqUserRegister(user_name, user_passwd);
    }
    else
    {
        ui->label_user_prompt->setText("未与服务器连接");
    }

    //不开始开始超时检测，但是计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 1;
    cloud_server_time_count_str.clear();
    cloud_server_time_count_str.append(prompt);
    cloud_server_time_out_cnt = 0;
}
//同步 action_num = 4
int MainWindow::bthUserSyncClicked()
{
    //取得参数
    QString user_name(ui->lineEdit_user_name->text());
    QString user_passwd(cloud_file_cfg.user_passwd);
    QString wkp(cloud_file_int->file_opt_int->current_user_path);
    QString empty;
    empty.clear();
    //提示语
    QString prompt;
    prompt.sprintf("提示：用户%s同步文件夹%s...", user_name.toLatin1().data(), wkp.toLatin1().data());
    ui->label_user_prompt->setText(prompt);
    ui->label_user_prompt->show();

    cloud_file_sync_type = 1;
     //连接服务器
    char tmp[64] = {0};
    ipCharToStr(tmp, cloud_file_cfg.server_ip);
    QString ip(tmp);
    qDebug()<<"连接测试： ip="<<ip<< "  port="<<cloud_file_cfg.port;
    cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);

    //发送信息
    if(cloud_file_int->client_sock_mark == 1)
    {
        cloud_file_int->reqSyncFile_start(cloud_file_cfg.user_name, cloud_file_cfg.user_passwd);
    }
    else
    {
        ui->label_user_prompt->setText("未与服务器连接");
    }

    //不开始开始超时检测，但是计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 1;
    cloud_server_time_count_str.clear();
    cloud_server_time_count_str.append(prompt);
    cloud_server_time_out_cnt = 0;
}
//注销 action_num = 4
int MainWindow::bthUserLogoutClicked()
{
    //服务器暂时没有此功能，在本地，实现的就是忘记密码
    ui->lineEdit_user_name->clear();
    ui->lineEdit_user_passwd->clear();
    memset(cloud_file_cfg.user_passwd, '\0', strlen(cloud_file_cfg.user_passwd));
    QString tmp;
    saveCfgFile_byName(cloud_file_int->file_opt_int->current_user_cfg_file, PARA_user_passwd, tmp);

}
//销毁 action_num = 6
//-------------密码再次验证--------------------------//
int MainWindow::UserDestory_passwdConfirm()
{
    QString passwd1(edit_passwd_1->text());
    QString passwd2(edit_passwd_2->text());
    if(passwd1.compare(passwd2) != 0)
    {
        user_destory_label_2->setText("两次的输入不一样！");
        user_destory_label_2->setStyleSheet("color:red;");
        return -1;
    }
    else
    {
        ui->lineEdit_user_passwd->setText(passwd1);
        /***密码加密**********************************/
        QString tmp(passwd1);
        char out[16];
        char tmp_str[64] = {0};
        strcpy(tmp_str, tmp.toLatin1().data());
        int len = tmp.length();
        if(len > 64)
        {
            len = 64;
        }
        dataEncrypt_byte8(tmp_str, len, out);
        strcpy(cloud_file_cfg.user_passwd, out);
        /********************************************/
        edit_passwd_1->clear();
        edit_passwd_2->clear();
        user_destory_diag->close();
    }
    return 0;
}
int MainWindow::UserDestory_passwdCancel()
{
    user_destory_cancel_mark = 1;
    user_destory_diag->close();
}
void MainWindow::UserDestory_diag_int()
{
    user_destory_diag = new QDialog(this);
    QVBoxLayout *layout= new QVBoxLayout(user_destory_diag);
    user_destory_diag->setWindowTitle("再次确认密码");

    QLabel *label_1 = new QLabel(user_destory_diag);
    label_1->setText("请输入你的密码");
    edit_passwd_1 = new QLineEdit(user_destory_diag);
    layout->addWidget(label_1);
    layout->addWidget(edit_passwd_1);
    edit_passwd_1->setEchoMode(QLineEdit::Password);


    user_destory_label_2 = new QLabel(user_destory_diag);
    user_destory_label_2->setText("再次输入你的密码");
    edit_passwd_2 = new QLineEdit(user_destory_diag);
    layout->addWidget(user_destory_label_2);
    layout->addWidget(edit_passwd_2);
    edit_passwd_2->setEchoMode(QLineEdit::Password);


    QPushButton *btnOk = new QPushButton(user_destory_diag);
    btnOk->setText("确定");
    QPushButton *btnCancel = new QPushButton(user_destory_diag);
    btnCancel->setText("取消");
    layout->addWidget(btnOk);
    layout->addWidget(btnCancel);

    connect(btnOk, SIGNAL(clicked()), this, SLOT(UserDestory_passwdConfirm()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(UserDestory_passwdCancel()));
}
//-----------------------------------------------------//
int MainWindow::bthUserDestoryClicked()
{
    //需要再次验证密码
    user_destory_cancel_mark = 0;
    user_destory_diag->exec();
    if(user_destory_cancel_mark == 1)
    {
        return -1;
    }

    //取得参数
    QString user_name(ui->lineEdit_user_name->text());
    QString user_passwd(cloud_file_cfg.user_passwd);

    //提示语
    QString prompt;
    prompt.sprintf("提示：用户%s销毁中...", user_name.toLatin1().data());
    ui->label_user_prompt->setText(prompt);
    ui->label_user_prompt->show();

     //连接服务器
    char tmp[64] = {0};
    ipCharToStr(tmp, cloud_file_cfg.server_ip);
    QString ip(tmp);
    qDebug()<<"连接服务器： ip="<<ip<< "  port="<<cloud_file_cfg.port;
    cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);

    //发送信息
    if(cloud_file_int->client_sock_mark == 1)
    {
        cloud_file_int->reqUserDestory(user_name, user_passwd);
    }
    else
    {
        ui->label_user_prompt->setText("未与服务器连接");
    }

    //不开始开始超时检测，但是计数
    cloud_server_time_out_en = 0;
    cloud_server_time_count_disp_en = 1;
    cloud_server_time_count_str.clear();
    cloud_server_time_count_str.append(prompt);
    cloud_server_time_out_cnt = 0;
}
//接收云端回复的信息并做相应的处理
int MainWindow::cloudFile_serverReply(int action_num, int reply_code, QString msg)
{
    QString tmp;
    qDebug()<<"=============>cloudFile_serverReply: action_num="<<action_num<<" reply_code="<<reply_code<<"  msg="<<msg;
    if(action_num == 1)//连接测试
    {
        cloud_server_time_count_disp_en = 0;
        tmp.clear();
        tmp.append("提示： 连接测试成功");
        //tmp.append(msg);
        ui->label_user_prompt->setText(tmp);
        ui->label_user_prompt->show();
    }
    else if(action_num == 2)//登录
    {
         cloud_server_time_count_disp_en = 0;
        tmp.clear();
        if(reply_code == 1)
        {
            tmp.append("提示： 登录成功");
        }
        else if(reply_code == -1)
        {
            tmp.append("提示： 登录失败，用户名不存在");
        }
        else if(reply_code == -2)
        {
            tmp.append("提示： 登录失败，密码错误");
        }
        else
        {
            tmp.append("提示： 登录失败，系统错误");
        }
         ui->label_user_prompt->setText(tmp);
        ui->label_user_prompt->show();
    }
    else if(action_num == 3)//注册
    {
        cloud_server_time_count_disp_en = 0;
        tmp.clear();
        if(reply_code == 1)
        {
            tmp.append("提示： 注册成功");
        }
        else if(reply_code == -1)
        {
            tmp.append("提示： 注册失败，用户名已存在");
        }
        else
        {
            tmp.append("提示： 注册失败，系统错误");
        }
         ui->label_user_prompt->setText(tmp);
        ui->label_user_prompt->show();

    }
    else if(action_num == 4 && cloud_file_sync_type == 1)//按键同步
    {
        cloud_server_time_count_disp_en = 0;
        tmp.clear();
        if(reply_code == 0)
        {
            ui->label_user_prompt->setStyleSheet("color:green;");
            tmp.append("提示： 同步阶段0");
        }
        else if(reply_code == 1)
        {
            ui->label_user_prompt->setStyleSheet("color:green;");
            tmp.append("提示： 同步阶段1");
        }
        else if(reply_code == 2)
        {
            ui->label_user_prompt->setStyleSheet("color:green;");
            tmp.append("提示： 同步阶段2");
        }
        else if(reply_code == 3)
        {
            ui->label_user_prompt->setStyleSheet("color:green;");
            tmp.append("提示： 同步完成");
        }
        else
        {
            ui->label_user_prompt->setStyleSheet("color:red;");
            tmp.append("提示： 系统错误");
        }
         ui->label_user_prompt->setText(tmp);
        ui->label_user_prompt->show();

    }
    else if(action_num == 4 && cloud_file_sync_type == 2)//自动同步
    {
        if(reply_code == 0)
        {
            ui->pushButton_cfg_autosync->setStyleSheet("background:white;");
            ui->pushButton_cfg_autosync->setText(">>>");
        }
        else if(reply_code == 1)
        {
            ui->pushButton_cfg_autosync->setStyleSheet("background:white;");
            ui->pushButton_cfg_autosync->setText(">>> >>>");
        }
        else if(reply_code == 2)
        {
            ui->pushButton_cfg_autosync->setStyleSheet("background:white;");
            ui->pushButton_cfg_autosync->setText(">>> >>> >>>");
        }
        else if(reply_code == 3)
        {
            ui->pushButton_cfg_autosync->setStyleSheet("background:green;");
            ui->pushButton_cfg_autosync->setText(" ");
        }
        else
        {
            ui->pushButton_cfg_autosync->setStyleSheet("background:red;");
        }
    }
    else if(action_num == 5)//注销
    {


    }
    else if(action_num == 6)//销毁
    {
        cloud_server_time_count_disp_en = 0;
        tmp.clear();
        if(reply_code == 1)
        {
            tmp.append("提示： 销毁成功");
        }
        else if(reply_code == -1)
        {
            tmp.append("提示： 销毁失败，用户名和密码验证失败");
        }
        else
        {
            tmp.append("提示： 销毁失败，系统错误");
        }
         ui->label_user_prompt->setText(tmp);
        ui->label_user_prompt->show();

    }


    return 0;
}
//超时处理
int MainWindow::cloudFile_timeoutPro()
{
    ui->label_user_prompt->setText("超时");
    cloud_server_time_out_en = 0;
}
//计时显示
int MainWindow::cloudFile_timeCountDisp()
{
    QString tmp1, tmp2;
    tmp1.setNum(cloud_server_time_out_cnt);
    tmp2.append(cloud_server_time_count_str);
    tmp2.append(tmp1);

    ui->label_user_prompt->setText(tmp2);
}


//设置自动同步
int MainWindow::cloudFile_autoSync(int en, int para)
{
    cloud_file_auto_sync_en = 1;
    cloud_file_auto_sync_para = para;
    return 0;
}
//一次同步
int MainWindow::cloudFile_oneShootSync(void)
{
    //连接服务器
   char tmp[64] = {0};
   ipCharToStr(tmp, cloud_file_cfg.server_ip);
   QString ip(tmp);
   cloud_file_int->connectTcpServer(ip, cloud_file_cfg.port);

   cloud_file_sync_type = 2;
   //发送信息
   if(cloud_file_int->client_sock_mark == 1)
   {
       cloud_file_int->reqSyncFile_start(cloud_file_cfg.user_name, cloud_file_cfg.user_passwd);
   }
   else
   {
       ui->pushButton_cfg_autosync->setStyleSheet("color:yellow;");
   }
    return 0;
}

/////////////////////////////////////////////////////////////////配置接口
int MainWindow::mydebug(void)
{
//    char a[4] = {0};
//    ipStrToChar("192.168.0.7", a);
//    qDebug()<<"==============>mydebug "<<(quint8)a[0]<<" "<<(quint8)a[1]<<" "<<(quint8)a[2]<<" "<<(quint8)a[3];

}

//取出配置文件里的信息
int MainWindow::cloudCfgFileGetInfo(QString file_name, strcut_CLOUD_FILE_CFG *s)
{
    //如果文件为空，或者不存在，则启用默认配置
    QFile f(file_name);
    if(f.exists() == false)
    {
        s->rem_passwd = 0;
        s->server_ip[0] = 0;
        s->server_ip[1] = 0;
        s->server_ip[2] = 0;
        s->server_ip[3] = 0;
        s->port = 0;
        s->auto_sync_para = 0;
        s->auto_sync_en = 0;
        s->startup_sync_en = 0;
        strcpy(s->user_name, "guest");
        char out[16] = {0};
        char ps[8] = "123";
        dataEncrypt_byte8(ps, 3, out);
        strcpy(s->user_passwd, out);

        return 0;
    }

    QString tmp;
    //rem_passwd
    getParaFromCfgFile_byName(file_name, PARA_rem_passwd, &tmp);
     s->rem_passwd = tmp.toInt();
    //server_ip[4]
    getParaFromCfgFile_byName(file_name, PARA_server_ip, &tmp);
    ipStrToChar(tmp.toLatin1().data(), s->server_ip);
    //port
    getParaFromCfgFile_byName(file_name, PARA_port, &tmp);
    s->port = tmp.toInt();
    //auto_sync_para
    getParaFromCfgFile_byName(file_name, PARA_auto_sync_para, &tmp);
    s->auto_sync_para = tmp.toInt();
    //auto_sync_en
    getParaFromCfgFile_byName(file_name, PARA_auto_sync_en, &tmp);
    s->auto_sync_en = tmp.toInt();
    //startup_sync_en
    getParaFromCfgFile_byName(file_name, PARA_startup_sync_en, &tmp);
    s->startup_sync_en = tmp.toInt();
   //user_name[64];
    if(0 != getParaFromCfgFile_byName(file_name, PARA_user_name, &tmp))
    {
        strcpy(s->user_name, "guest");
    }
    else
    {
        strcpy(s->user_name, tmp.toLatin1().data());
    }

    //user_passwd[64];
    if(0 != getParaFromCfgFile_byName(file_name, PARA_user_passwd, &tmp))
    {
        char out[16] = {0};
        char ps[8] = "123";
        dataEncrypt_byte8(ps, 3, out);
        strcpy(s->user_passwd, out);
    }
    else
    {
        strcpy(s->user_passwd, tmp.toLatin1().data());
    }

}
//显示
int MainWindow::scloudCfgInfoShow(strcut_CLOUD_FILE_CFG s)
{
    qDebug()<<"======================= scloudCfgInfoShow =================";
    qDebug()<<"rem_passwd="<<s.rem_passwd;
    qDebug()<<"server_ip="<<(quint8)s.server_ip[0]<<"."<<(quint8)s.server_ip[1]<<"."<<(quint8)s.server_ip[2]<<"."<<(quint8)s.server_ip[3];
    qDebug()<<"port="<<s.port;
    qDebug()<<"auto_sync_para="<<s.auto_sync_para;
    qDebug()<<"auto_sync_en="<<s.auto_sync_en;
    qDebug()<<"startup_sync_en="<<s.startup_sync_en;
    qDebug()<<"user_name="<<QString(s.user_name);
    qDebug()<<"user_passwd="<<QString(s.user_passwd);
    return 0;
}
//配置写入配置文件
int MainWindow::cloudCfgSaveToFile(QString file_name, strcut_CLOUD_FILE_CFG s)
{
    QString tmp;
    //rem_passwd
    tmp.sprintf("%d", s.rem_passwd);
    saveCfgFile_byName(file_name, PARA_rem_passwd, tmp);
    //server_ip[4]
    tmp.sprintf("%d.%d.%d.%d", (quint8)s.server_ip[0], (quint8)s.server_ip[1], (quint8)s.server_ip[2], (quint8)s.server_ip[3]);
    saveCfgFile_byName(file_name, PARA_server_ip, tmp);
    //port
    tmp.sprintf("%d", s.port);
    saveCfgFile_byName(file_name, PARA_port, tmp);
    //auto_sync_para
    tmp.sprintf("%d", s.auto_sync_para);
    saveCfgFile_byName(file_name, PARA_auto_sync_para, tmp);
    //auto_sync_en
    tmp.sprintf("%d", s.auto_sync_en);
    //qDebug()<<"cloudCfgSaveToFile: PARA_auto_sync_en="<<tmp<<" s.auto_sync_en"<<s.auto_sync_en;
    saveCfgFile_byName(file_name, PARA_auto_sync_en, tmp);
    //startup_sync_en
    tmp.sprintf("%d", s.startup_sync_en);
    saveCfgFile_byName(file_name, PARA_startup_sync_en, tmp);
   //user_name[64];
    tmp.clear();
    tmp.append(s.user_name);
    saveCfgFile_byName(file_name, PARA_user_name, tmp);
    //user_passwd[64];
    tmp.clear();
    tmp.append(s.user_passwd);
    saveCfgFile_byName(file_name, PARA_user_passwd, tmp);

    return 0;
}
//执行配置内容（显示到面板上）
int MainWindow::cloudCfgExe(strcut_CLOUD_FILE_CFG s)
{

    QString tmp;
    cloud_file_cfg = s;
    //rem_passwd
    if(s.rem_passwd == 1)
    {
        ui->checkBox_passwd_rem->setChecked(true);
    }
    else
    {
        ui->checkBox_passwd_rem->setChecked(false);
    }
    //server_ip[4]
    tmp.sprintf("%d.%d.%d.%d", (quint8)s.server_ip[0], (quint8)s.server_ip[1], (quint8)s.server_ip[2], (quint8)s.server_ip[3]);
    ui->lineEdit_cfg_ip->setText(tmp);
    //port
    tmp.sprintf("%d", s.port);
    ui->lineEdit_cfg_port->setText(tmp);
    //auto_sync_para
    tmp.sprintf("%d", s.auto_sync_para);
    ui->lineEdit_cfg_autosync->setText(tmp);
    //auto_sync_en
    if(1 == s.auto_sync_en)
    {
        ui->checkBox_cfg_autonsync->setChecked(true);
        cloudFile_autoSync(1, cloud_file_cfg.auto_sync_para);
        ui->pushButton_cfg_autosync->show();

    }
    else
    {
        ui->checkBox_cfg_autonsync->setChecked(false);
        ui->pushButton_cfg_autosync->hide();
    }
    //startup_sync_en
    if(1 == s.startup_sync_en)
    {
        ui->checkBox_cfg_power_on_sync->setChecked(true);
    }
    else
    {
        ui->checkBox_cfg_power_on_sync->setChecked(false);
    }
   //user_name[64];
    tmp.clear();
    tmp.append(s.user_name);
    ui->lineEdit_user_name->setText(tmp);
    //user_passwd[64];
    if(s.rem_passwd == 1)
    {
         cfg_passwd_change_en = 0;
         ui->lineEdit_user_passwd->setText("********");
         cfg_passwd_change_en = 1;
    }
    else
    {
         ui->lineEdit_user_passwd->clear();
    }
    cfg_passwd_change_mark = 0;

    return 0;
}

//从面板中获取配置信息
int MainWindow::cloudCfgGetBaordCfgInfo(strcut_CLOUD_FILE_CFG *s)
{
    QString tmp;
    //rem_passwd
    if(ui->checkBox_passwd_rem->checkState() == Qt::Checked)
    {
        s->rem_passwd = 1;
    }
    else
    {
        s->rem_passwd = 0;
    }
    //server_ip[4]
    tmp = ui->lineEdit_cfg_ip->text();
    ipStrToChar(tmp.toLatin1().data(), s->server_ip);
    //port
    tmp = ui->lineEdit_cfg_port->text();
    s->port = tmp.toInt();
    //auto_sync_para
    tmp = ui->lineEdit_cfg_autosync->text();
    s->auto_sync_para = tmp.toInt();
    //auto_sync_en
    if(ui->checkBox_cfg_autonsync->checkState() == Qt::Checked)
    {
        s->auto_sync_en = 1;
    }
    else
    {
        s->auto_sync_en = 0;
    }
    //qDebug()<<"cloudCfgGetBaordCfgInfo: auto_sync_en"<<s->auto_sync_en;
    //startup_sync_en
    if(ui->checkBox_cfg_power_on_sync->checkState() == Qt::Checked)
    {
        s->startup_sync_en = 1;
    }
    else
    {
        s->startup_sync_en = 0;
    }
   //user_name[64];
    tmp = ui->lineEdit_user_name->text();
    strcpy(s->user_name, tmp.toLatin1().data());
    //user_passwd[64]; 注：只有用户输入密码时，才动作，所以要有 文本改动的标记  textChanged()
//    qDebug()<<"cfg_passwd_change_mark="<<cfg_passwd_change_mark;
//    if(ui->checkBox_passwd_rem->checkState() == Qt::Checked && cfg_passwd_change_mark == 1)
//    {
//        cfg_passwd_change_mark = 0;
//        tmp = ui->lineEdit_user_passwd->text();
//        char out[16];
//        char tmp_str[64] = {0};
//        strcpy(tmp_str, tmp.toLatin1().data());
//        int len = tmp.length();
//        if(len > 64)
//        {
//            len = 64;
//        }
//        dataEncrypt_byte8(tmp_str, len, out);
//        strcpy(s->user_passwd, out);
//    }
//    else
//    {
//        //memset(s->user_passwd, '\0', strlen(s->user_passwd));
//    }


    return 0;
}

//cfg 更改未保存提示
int MainWindow::cfgCfgNotSavePrompt()
{
    ui->label_cfg_prompt->setText("您有更改却未保存的信息");
    ui->label_cfg_prompt->setStyleSheet("color:red;");
    ui->label_cfg_prompt->show();
    return 0;
}
//密码更改动作
int MainWindow::cfgUserPasswdChanged()
{
    if(cfg_passwd_change_en != 1)
    {
        return 0;
    }

    cfg_passwd_change_mark = 1;
    QString tmp;
    tmp = ui->lineEdit_user_passwd->text();
    char out[16];
    char tmp_str[64] = {0};
    strcpy(tmp_str, tmp.toLatin1().data());
    int len = tmp.length();
    if(len > 64)
    {
        len = 64;
    }
    dataEncrypt_byte8(tmp_str, len, out);
    strcpy(cloud_file_cfg.user_passwd, out);
    //qDebug()<<"checkBox_passwd_rem="<<ui->checkBox_passwd_rem->checkState();
    return 0;
}








