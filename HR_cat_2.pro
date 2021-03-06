#-------------------------------------------------
#
# Project created by QtCreator 2018-11-15T20:19:05
#
#-------------------------------------------------

QT       += core gui
QT       +=core gui printsupport
QT      +=charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT_CONFIG -= no-pkg-config
CONFIG  += link_pkgconfig
PKGCONFIG += opencv
PKGCONFIG+= x11
#for gsl
INCLUDEPATH += /usr/local/Cellar/gsl/2.5/include
INCLUDEPATH += /usr/local/include
INCLUDEPATH +=/opt/X11/include
INCLUDEPATH +=/Users/yanyupeng/Documents/xmu/dlib
LIBS+=/usr/local/Cellar/gsl/2.5/lib/libgsl.a
LIBS+=/usr/local/Cellar/gsl/2.5/lib/libgslcblas.a
LIBS+=/usr/local/lib/libfftw3.a
LIBS+=/Users/yanyupeng/Documents/xmu/dlib/examples/build/dlib_build/libdlib.a
LIBS+= -L/opt/X11/lib
LIBS+= -L/Users/yanyupeng/Documents/xmu/dlib/dlib

TARGET = HR_cat_2
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        hr_cat_2.cpp \
    qcustomplot.cpp

HEADERS += \
        hr_cat_2.h \
    qcustomplot.h

FORMS += \
        hr_cat_2.ui

RESOURCES += \
    1.qrc \
    1.qrc

DISTFILES +=
