#-------------------------------------------------
#
# Project created by QtCreator 2014-07-22T17:13:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VidExtractor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

CONFIG += link_pkgconfig
PKGCONFIG += opencv
