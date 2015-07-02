#-------------------------------------------------
#
# Project created by QtCreator 2015-06-12T16:22:44
#
#-------------------------------------------------

QT       -= core gui

TARGET = pclsync
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD/../lib/mbedtls/include/
INCLUDEPATH += $$PWD/../lib/sqlite
INCLUDEPATH += $$PWD/../lib/fuse2cbfs

DEFINES += P_OS_WINDOWS
DEFINES += P_SSL_MBEDTLS

SOURCES += \
    ../pupload.c \
    ../publiclinks.c \
    ../ptree.c \
    ../ptimer.c \
    ../ptasks.c \
    ../psynclib.c \
    ../psyncer.c \
    ../pstatus.c \
    ../pssl-mbedtls.c \
    ../pssl.c \
    ../psettings.c \
    ../pscanner.c \
    ../prunratelimit.c \
    ../ppassword.c \
    ../ppagecache.c \
    ../pp2p.c \
    ../pnotifications.c \
    ../pnetlibs.c \
    ../pmemlock.c \
    ../plocks.c \
    ../plocalscan.c \
    ../plocalnotify.c \
    ../plist.c \
    ../plibs.c \
    ../pintervaltree.c \
    ../pfsxattr.c \
    ../pfsupload.c \
    ../pfstasks.c \
    ../pfsstatic.c \
    ../pfsfolder.c \
    ../pfscrypto.c \
    ../pfs.c \
    ../pfolder.c \
    ../pfileops.c \
    ../pexternalstatus.c \
    ../pdownload.c \
    ../pdiff.c \
    ../pcrypto.c \
    ../pcrc32c.c \
    ../pcontacts.c \
    ../pcompat.c \
    ../pcloudcrypto.c \
    ../pcallbacks.c \
    ../pcache.c \
    ../pbusinessaccount.c \
    ../papi.c \
    ../cli.c \
    ../poverlay.c

HEADERS += \
    ../pupload.h \
    ../publiclinks.h \
    ../ptypes.h \
    ../ptree.h \
    ../ptimer.h \
    ../ptasks.h \
    ../psynclib.h \
    ../psyncer.h \
    ../pstatus.h \
    ../pssl-mbedtls.h \
    ../psslcerts.h \
    ../pssl.h \
    ../psettings.h \
    ../pscanner.h \
    ../pscanexts.h \
    ../prunratelimit.h \
    ../ppassworddict.h \
    ../ppassword.h \
    ../ppagecache.h \
    ../pp2p.h \
    ../pnotifications.h \
    ../pnetlibs.h \
    ../pmemlock.h \
    ../plocks.h \
    ../plocalscan.h \
    ../plocalnotify.h \
    ../plist.h \
    ../plibs.h \
    ../pintervaltree.h \
    ../pfsxattr.h \
    ../pfsupload.h \
    ../pfstasks.h \
    ../pfsstatic.h \
    ../pfsfolder.h \
    ../pfscrypto.h \
    ../pfs.h \
    ../pfolder.h \
    ../pfileops.h \
    ../pexternalstatus.h \
    ../pdownload.h \
    ../pdiff.h \
    ../pdatabase.h \
    ../pcrypto.h \
    ../pcrc32c.h \
    ../pcontacts.h \
    ../pcompiler.h \
    ../pcompat.h \
    ../pcloudcrypto.h \
    ../pcallbacks.h \
    ../pcache.h \
    ../pbusinessaccount.h \
    ../papi.h \
    ../poverlay.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
