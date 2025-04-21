QT       += core gui
QT += core gui widgets network
QT += core gui widgets network core5compat
QT += widgets network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    genderselectdialog.cpp \
    main.cpp \
    mainwindow.cpp

# 添加新文件到项目
HEADERS += \
    genderselectdialog.h \
    mainwindow.h \
    hobbyselectdialog.h  # 新增

SOURCES += \
    genderselectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    hobbyselectdialog.cpp  # 新增项放在这里

HEADERS += \
    genderselectdialog.h \
    mainwindow.h \
    hobbyselectdialog.h  # 新增

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
