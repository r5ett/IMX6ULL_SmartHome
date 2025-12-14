/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_ir;
    QLabel *label_7;
    QLabel *label_9;
    QLabel *label_light;
    QLabel *label_dis;
    QLabel *label_tmp;
    QLabel *label_hum;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1024, 600);
        MainWindow->setMinimumSize(QSize(1024, 600));
        MainWindow->setMaximumSize(QSize(1024, 600));
        QFont font;
        font.setPointSize(20);
        MainWindow->setFont(font);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(0, 20, 1024, 61));
        label->setMinimumSize(QSize(1024, 0));
        label->setMaximumSize(QSize(1024, 16777215));
        QFont font1;
        font1.setPointSize(24);
        font1.setBold(true);
        font1.setWeight(75);
        label->setFont(font1);
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(260, 110, 240, 50));
        QFont font2;
        font2.setPointSize(25);
        pushButton->setFont(font2);
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(521, 110, 240, 50));
        pushButton_2->setFont(font2);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(220, 200, 67, 41));
        label_3->setFont(font);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(550, 200, 67, 41));
        label_4->setFont(font);
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(640, 290, 67, 41));
        label_5->setFont(font);
        label_ir = new QLabel(centralwidget);
        label_ir->setObjectName(QString::fromUtf8("label_ir"));
        label_ir->setGeometry(QRect(300, 200, 200, 40));
        label_ir->setFont(font);
        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(150, 290, 80, 40));
        label_7->setFont(font);
        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(390, 290, 80, 40));
        label_9->setFont(font);
        label_light = new QLabel(centralwidget);
        label_light->setObjectName(QString::fromUtf8("label_light"));
        label_light->setGeometry(QRect(630, 200, 200, 40));
        label_light->setFont(font);
        label_dis = new QLabel(centralwidget);
        label_dis->setObjectName(QString::fromUtf8("label_dis"));
        label_dis->setGeometry(QRect(730, 290, 200, 40));
        label_dis->setFont(font);
        label_tmp = new QLabel(centralwidget);
        label_tmp->setObjectName(QString::fromUtf8("label_tmp"));
        label_tmp->setGeometry(QRect(230, 290, 100, 40));
        label_tmp->setFont(font);
        label_hum = new QLabel(centralwidget);
        label_hum->setObjectName(QString::fromUtf8("label_hum"));
        label_hum->setGeometry(QRect(470, 290, 100, 40));
        label_hum->setFont(font);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1024, 37));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\"><span style=\" color:#000000;\">\345\237\272\344\272\216IMX6ULL\347\232\204\346\231\272\350\203\275\345\256\266\345\261\205\347\263\273\347\273\237</span></p></body></html>", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "\347\201\257\345\205\211", nullptr));
        pushButton_2->setText(QApplication::translate("MainWindow", "\351\227\250\351\223\203", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\347\272\242\345\244\226: </p></body></html>", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\345\205\211\347\205\247: </p></body></html>", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\350\267\235\347\246\273: </p></body></html>", nullptr));
        label_ir->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">0</p></body></html>", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\346\270\251\345\272\246 :</p></body></html>", nullptr));
        label_9->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\346\271\277\345\272\246 :</p></body></html>", nullptr));
        label_light->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">0</p></body></html>", nullptr));
        label_dis->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">0</p></body></html>", nullptr));
        label_tmp->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">0</p></body></html>", nullptr));
        label_hum->setText(QApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">0</p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
