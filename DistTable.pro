#-------------------------------------------------
#
# Project created by QtCreator 2015-03-25T18:17:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DistTable
TEMPLATE = app

QMAKE_CXXFLAGS += -std=gnu++11

macx:QMAKE_CXXFLAGS += -stdlib=libc++
macx:QMAKE_LFLAGS += -stdlib=libc++

SOURCES += main.cpp\
        window.cpp \
    spreadmanager.cpp \
    notificationmanager.cpp \
    canvas.cpp \
    line.cpp

HEADERS  += window.h \
    spreadmanager.h \
    singleton.h \
    notificationmanager.h \
    canvas.h \
    line.h

FORMS    += window.ui

INCLUDEPATH += $$PWD/lib/
INCLUDEPATH += $$PWD/include/

DEPENDPATH += $$PWD/lib/

PRE_TARGETDEPS += $$PWD/lib/libspread.a
PRE_TARGETDEPS += $$PWD/lib/libtspread-core.a
PRE_TARGETDEPS += $$PWD/lib/libspread-core.a

LIBS += -L$$PWD/lib/ -lspread
LIBS += -L$$PWD/lib/ -ltspread-core
LIBS += -L$$PWD/lib/ -lspread-core


_QJSON_PATH = /usr/local/Cellar/qjson/0.8.1
INCLUDEPATH += "$${_QJSON_PATH}/include/"
LIBS += -L$${_QJSON_PATH}/lib


OTHER_FILES += \
    $$PWD/spread_daemon/spread \
    $$PWD/spread_daemon/spread.conf


# copy spread daemon and config file

copydata.commands = $(COPY_DIR) $$PWD/spread_daemon $$OUT_PWD/spread
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata
