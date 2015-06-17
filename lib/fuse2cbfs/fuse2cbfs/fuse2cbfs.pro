#-------------------------------------------------
#
# Project created by QtCreator 2015-06-12T14:48:27
#
#-------------------------------------------------

QT       -= core gui

TARGET = fuse2cbfs
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ../fuse2cbfs.cpp

HEADERS += \
    ../fuse2cbfs.h \
    ../CbFS.h \
    ../cbfscab.h \
    ../fuse.h

LIBS += -L$$PWD/../ -lcbfs -lversion -lsetupapi -lnetapi32 -lole32

INCLUDEPATH += $$PWD/../
DEPENDPATH += $$PWD/../
