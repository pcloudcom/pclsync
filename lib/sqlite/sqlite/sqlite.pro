#-------------------------------------------------
#
# Project created by QtCreator 2015-06-12T15:05:18
#
#-------------------------------------------------

QT       -= core gui

TARGET = sqlite
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ../sqlite3.c

HEADERS += \
    ../targetver.h \
    ../sqlite3.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
