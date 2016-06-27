QT += core network
QT -= gui

CONFIG += c++11

TARGET = rfcommserver
CONFIG += console
CONFIG -= app_bundle
LIBS += -lbluetooth

TEMPLATE = app

SOURCES += main.cpp \
    rfcommserver.cpp \
    clienthandler.cpp

HEADERS += \
    rfcommserver.h \
    clienthandler.h
