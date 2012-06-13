QT       += core gui opengl

TARGET = Planets3D
TEMPLATE = app


SOURCES += main.cpp\
    planet.cpp \
    camera.cpp \
    mainwindow.cpp \
    planetswidget.cpp

HEADERS  += \
    planet.h \
    common.h \
    camera.h \
    mainwindow.h \
    planetswidget.h

FORMS += \
    mainwindow.ui



