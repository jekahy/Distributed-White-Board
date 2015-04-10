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
    spreadmanager.cpp

HEADERS  += window.h \
    spreadmanager.h

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

#LIBS += -lQt5Concurrent
