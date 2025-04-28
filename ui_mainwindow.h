/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEditChat;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEditInput;
    QPushButton *pushButtonSend;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        MainWindow->setStyleSheet(QString::fromUtf8("\n"
"    QMainWindow { background-color: white; }\n"
"   "));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        textEditChat = new QTextEdit(centralwidget);
        textEditChat->setObjectName("textEditChat");
        textEditChat->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textEditChat->setReadOnly(true);
        QFont font;
        font.setPointSize(18);
        textEditChat->setFont(font);
        textEditChat->setStyleSheet(QString::fromUtf8("background-color: white;"));

        verticalLayout->addWidget(textEditChat);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(20);
        horizontalLayout->setObjectName("horizontalLayout");
        lineEditInput = new QLineEdit(centralwidget);
        lineEditInput->setObjectName("lineEditInput");
        lineEditInput->setMinimumSize(QSize(400, 60));
        lineEditInput->setFont(font);

        horizontalLayout->addWidget(lineEditInput);

        pushButtonSend = new QPushButton(centralwidget);
        pushButtonSend->setObjectName("pushButtonSend");
        pushButtonSend->setFont(font);
        pushButtonSend->setMinimumSize(QSize(150, 60));

        horizontalLayout->addWidget(pushButtonSend);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);
        QObject::connect(lineEditInput, &QLineEdit::returnPressed, pushButtonSend, qOverload<>(&QPushButton::click));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "AiChat", nullptr));
        lineEditInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\257\267\350\276\223\345\205\245\346\226\207\345\255\227", nullptr));
        pushButtonSend->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
