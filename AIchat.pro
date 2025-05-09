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
    customizepage.cpp \
    genderselectdialog.cpp \
    hobbyselectdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    markdownparser.cpp \
    topics_data.cpp

HEADERS += \
    customizepage.h \
    genderselectdialog.h \
    hobbyselectdialog.h \
    mainwindow.h \
    markdownparser.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = logo.ico

