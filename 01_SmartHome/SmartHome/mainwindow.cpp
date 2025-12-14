#include "mainwindow.h"
#include "ui_mainwindow.h"

#include </usr/include/stdio.h>
#include </usr/include/fcntl.h>   // 定义 O_RDWR、O_RDONLY、O_WRONLY 等标志
#include </usr/include/unistd.h>  // 定义 open()、close() 等系统调用函数
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /* 加载UI */
    ui->setupUi(this);

    /*******************  led  ******************/
    /* 自动加载驱动 */
    if (!QFile::exists("/dev/r5ett_led")) {
        qDebug() << "检测到 /dev/r5ett_led 不存在，正在自动加载驱动...";
        QProcess::execute("insmod", QStringList() << "/root/SmartHome/02_led/leddrv.ko");
        // 等待一下让驱动初始化完成
        QThread::msleep(200);
    }
    // 打开我们之前写好的 LED 驱动
    led_fd = open("/dev/r5ett_led", 2);
    if (led_fd < 0) {
        qDebug() << "打开 /dev/r5ett_led 失败！请先 insmod leddrv.ko";
        ui->pushButton->setEnabled(false);  // 打开失败就禁用按钮变灰
    }
    else {
        qDebug() << "LED 驱动打开成功！";
        // 可选：启动时默认关灯
        buf[0] = 0;
        write(led_fd, buf, 1);
    }

    /*******************  dht11  ******************/
    if (!QFile::exists("/dev/r5ett_dht11")) {
        qDebug() << "检测到 /dev/r5ett_dht11 不存在，正在自动加载驱动...";
        QProcess::execute("insmod", QStringList() << "/root/SmartHome/03_dht11/dht11drv.ko");
        // 等待一下让驱动初始化完成
        QThread::msleep(200);
    }

    dht11_fd = open("/dev/r5ett_dht11", O_RDONLY);
    if (dht11_fd < 0) {
        qDebug() << "打开 /dev/r5ett_dht11 失败！请检查驱动是否加载";
    }
    else {
        qDebug() << "DHT11 设备打开成功";
        read_dht11();  // 启动时立刻读一次
    }

    dht11_timer = new QTimer(this);
    dht11_timer->setInterval(3000);  // 每 3 秒读一次（DHT11 不能读太频繁）
    connect(dht11_timer, &QTimer::timeout, this, &MainWindow::read_dht11);
    dht11_timer->start();


    /*******************  ap3216c  ******************/
    if (!QFile::exists("/dev/r5ett_ap3216c")) {
        qDebug() << "检测到 /dev/r5ett_ap3216c 不存在，正在自动加载驱动...";
        QProcess::execute("insmod", QStringList() << "/root/SmartHome/04_ap3216c/ap3216cdrv.ko");
        // 等待一下让驱动初始化完成
        QThread::msleep(200);
    }

    // 打开 AP3216C 设备
    ap3216_fd = open(ap3216_dev.toStdString().c_str(), O_RDWR);
    if (ap3216_fd < 0) {
        qDebug() << "打开 /dev/r5ett_ap3216c 失败！请检查驱动是否正确加载";
        ui->label_light->setText("ALS: ---");
        ui->label_ir->setText("IR: ---");
    }
    else {
        qDebug() << "AP3216C 设备打开成功";
        // open() 会自动触发一次初始化（你驱动里有写 0x03），不需要再写
    }

    ap3216c_tim = new QTimer();/* 创建一个名为ap3216c_tim的QTimer定时器对象 */
    ap3216c_tim->setInterval(300);/* 给ap3216c_tim定时器设置触发间隔为300毫秒 */
    /* 每300ms，ap3216c_tim触发timeout()信号→自动调用ap3216_timeout()函数。 */
    connect(ap3216c_tim, &QTimer::timeout, this, &MainWindow::ap3216_timeout);
    ap3216c_tim->start();

    /*******************  sr04  ******************/
    if (!QFile::exists("/dev/r5ett_sr04")) {
        qDebug() << "检测到 /dev/r5ett_sr04 不存在，正在自动加载驱动...";
        QProcess::execute("insmod", QStringList() << "/root/SmartHome/05_sr04/sr04drv.ko");
        // 等待一下让驱动初始化完成
        QThread::msleep(200);
    }

    sr04_fd = open("/dev/r5ett_sr04", O_RDONLY);
    if (sr04_fd < 0) {
        qDebug() << "打开 /dev/r5ett_sr04 失败！请检查驱动是否加载";
    }
    else {
        qDebug() << "sr04 设备打开成功";
        read_sr04();  // 启动时立刻读一次
    }

    sr04_timer = new QTimer(this);
    sr04_timer->setInterval(300);  // 每 0.3 秒读一次
    connect(sr04_timer, &QTimer::timeout, this, &MainWindow::read_sr04);
    sr04_timer->start();

    /*******************  beep  ******************/
    /* 自动加载驱动 */
    if (!QFile::exists("/dev/beep")) {
        qDebug() << "检测到 /dev/beep 不存在，正在自动加载驱动...";
        QProcess::execute("insmod", QStringList() << "/root/SmartHome/06_beep/beepdrv.ko");
        // 等待一下让驱动初始化完成
        QThread::msleep(200);
    }
    // 打开我们之前写好的 beep 驱动
    beep_fd = open("/dev/beep", 2);
    if (beep_fd < 0) {
        qDebug() << "打开 /dev/beep 失败！请先 insmod beepdrv.ko";
        ui->pushButton->setEnabled(false);  // 打开失败就禁用按钮变灰
    }
    else {
        qDebug() << "beep 驱动打开成功！";
        // 可选：启动时默认关灯
        buf[0] = 1;
        write(beep_fd, buf, 1);
    }

    beep_tim = new QTimer();
    beep_tim->setInterval(1000);
    connect(beep_tim,SIGNAL(timeout()),this,SLOT(beep_timeout()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/* 蜂鸣器的定时关闭 */
void MainWindow::beep_timeout()
{
    /* disable beep */
    buf[0] = 0;
    write(beep_fd, buf, 1);
    /* stop beep_tim */
    beep_tim->stop();
}

/* dht11 */
void MainWindow::read_dht11()
{
    if (dht11_fd < 0) return;

    if (read(dht11_fd, dht11_buf, 4) != 4) {
        qDebug() << "DHT11 读取失败";
        return;
    }

    // 解析数据并显示（带一位小数）
    double humidity = dht11_buf[0] + dht11_buf[1] / 10.0;
    double temperature = dht11_buf[2] + dht11_buf[3] / 10.0;

    ui->label_hum->setText(QString::number(humidity, 'f', 1) + " %");
    ui->label_tmp->setText(QString::number(temperature, 'f', 1) + " ℃");

    qDebug() << "DHT11:" << temperature << "℃" << humidity << "%";
}

/* ap3216c */
void MainWindow::ap3216_timeout()
{
    if (ap3216_fd < 0) return;

    unsigned char buf[6];
    if (read(ap3216_fd, buf, 6) != 6) {
        qDebug() << "AP3216C 读取失败";
        return;
    }

    // 组合成 16bit 数据（低字节在前）
    uint16_t ir_raw  = buf[0] | (buf[1] << 8);   // 0x0A, 0x0B
    uint16_t als_raw = buf[2] | (buf[3] << 8);   // 0x0C, 0x0D

    // 判断数据有效性（bit7 为 0 才有效）
    bool ir_valid  = (ir_raw  & 0x8000) == 0;
    bool als_valid = (als_raw & 0x8000) == 0;

    // 取有效数据位
    int ir  = ir_valid  ? (ir_raw  & 0x3FF) : -1;   // 10bit
    int als = als_valid ? (als_raw & 0xFFFF) : -1;  // 16bit

    // ====== 显示到 QLabel（防闪烁：只在数值变化时更新）======
    if (als != last_als && als >= 0) {
        last_als = als;
        // 简单映射到大致勒克斯（lux），AP3216C 官方近似公式
        double lux = als * 0.28;  // 经验值，不同光照环境略有差异
        if (lux > 1000) lux = 1000;  // 超过 1000lux 就显示 1000+
        ui->label_light->setText(QString("%1 lux").arg(lux, 0, 'f', 0));
    }

    if (ir != last_ir && ir >= 0) {
        last_ir = ir;
        ui->label_ir->setText(QString("%1").arg(ir));
    }
}

/* sr04 */
void MainWindow::read_sr04()
{
    if (sr04_fd < 0) {
        ui->label_dis->setText("距离: ---");
        return;
    }

    uint64_t time_ns = 0;
    if (read(sr04_fd, &time_ns, 4) == 4 && time_ns > 0) {
        int distance_cm = (int)(time_ns / 58823);  // 直接算距离（cm）
        if (distance_cm > 400) distance_cm = 400;
        ui->label_dis->setText(QString("距离: %1 cm").arg(distance_cm));
    } else {
        ui->label_dis->setText("距离: ---");
    }
}

/* control led */
void MainWindow::on_pushButton_clicked()
{
    // 读取当前灯的状态（可选，也可以不读，直接取反）
    read(led_fd, buf, 1);

    // 状态取反：0→1（开灯），1→0（关灯）
    buf[0] = !buf[0];

    // 写回新状态
    write(led_fd, buf, 1);

    // 可选：按钮文字实时显示当前状态
    if (buf[0] == 1) {
        ui->pushButton->setText("灯光（已开）");
        ui->pushButton->setStyleSheet("background-color: yellow; color: black;");
    } else {
        ui->pushButton->setText("灯光");
        ui->pushButton->setStyleSheet("");
    }
}

/* control beep */
void MainWindow::on_pushButton_2_clicked()
{
    if (beep_fd < 0) {
            qDebug() << "beep device not opened";
            return;
        }

    unsigned char on  = 0;  // beepON
    unsigned char off = 1;  // beepOFF

    /* 1. 蜂鸣器响 */
    write(beep_fd, &on, 1);
    QThread::msleep(200);
    write(beep_fd, &off, 1);
    beep_tim->stop();
}

