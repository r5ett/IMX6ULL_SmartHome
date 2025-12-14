#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    //void on_pushButton_3_clicked();

    void read_dht11();

    void read_sr04();

public slots:
    void ap3216_timeout();
    void beep_timeout();


private:
    Ui::MainWindow *ui;
    /* 硬件设备文件描述符 */
    int led_fd, beep_fd, ap3216_fd, dht11_fd,sr04_fd;

    /* 设备文件路径 */
    QString led_dev = "/dev/r5ett_led";
    QString beep_dev = "/dev/beep";
    QString ap3216_dev = "/dev/r5ett_ap3216c";
    QString dht11_dev = "/dev/r5ett_dht11";

    /* 存取设备中读取的数据 */
    unsigned char buf[10];
    unsigned char dht11_buf[5]; // 专门存 DHT11 的 5 字节原始数据
    unsigned char sr04_buf;

    // 用于保存上一次的有效值，防止闪烁
    int last_als = -1;
    int last_ir  = -1;
    int last_distance = -1;

    QTimer *ap3216c_tim;
    QTimer *beep_tim;
    QTimer *dht11_timer; // 定时读取 DHT11
    QTimer *sr04_timer;

};
#endif // MAINWINDOW_H
