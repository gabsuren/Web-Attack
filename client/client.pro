#-------------------------------------------------
#
# Project created by QtCreator 2015-09-06T15:03:26
#
#-------------------------------------------------

QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
	clientwindow.cpp \

HEADERS  += mainwindow.h\
	    clientwindow.h \

FORMS    += mainwindow.ui \
    clientwindow.ui
