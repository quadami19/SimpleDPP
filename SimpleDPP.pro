#-------------------------------------------------
#
# Project created by QtCreator 2017-09-19T15:24:46
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia widgets
QT       += serialport
CONFIG   += c++11
RESOURCES += res.qrc

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = SimpleDPP
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    audiodetector.cpp \
    inputdevicesetdialog.cpp \
    interpolatingdialog.cpp \
    processing.cpp \
    core.cpp \
    pulseshapewidget.cpp \
    processingdialog.cpp \
    neuralnetsettingsdialog.cpp \
    nuclearphysicsperceptron.cpp \
    teachingclass.cpp \
    perceptron.cpp \
    neuron_base.cpp \
    qcustomplot/qcustomplot.cpp \
    nuclteachingclass.cpp \
    streamsmanagerdialog.cpp \
    serialport.cpp \
    filtering.cpp \
    filteringdialog.cpp \
    debugmenu.cpp \
    autosavedialog.cpp \
    interpolator.cpp \
    processingsettings.cpp

HEADERS  += mainwindow.hpp \
   audiodetector.hpp \
    inputdevicesetdialog.hpp \
    interpolatingdialog.hpp \
    processing.hpp \
    core.hpp \
    pulseshapewidget.hpp \
    processingdialog.hpp \
    neuralnetsettingsdialog.hpp \
    nuclearphysicsperceptron.hpp \
    teachingclass.hpp \
    perceptron.hpp \
    neuron_base.hpp \
    qcustomplot/qcustomplot.h \
    fft.hpp \
    nuclteachingclass.hpp \
    streamsmanagerdialog.hpp \
    serialport.hpp \
    filtering.hpp \
    filteringdialog.hpp \
    debugmenu.hpp \
    autosavedialog.hpp \
    datum_types.hpp \
    interpolator.hpp \
    processingsettings.hpp


