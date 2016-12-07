#-------------------------------------------------
#
# Project created by QtCreator 2014-07-28T08:46:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VidsExtractor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    widgetcam.cpp

HEADERS  += mainwindow.h \
    widgetcam.h

CONFIG += link_pkgconfig
PKGCONFIG += opencv
