#-------------------------------------------------
#
# Project created by QtCreator 2018-10-21T14:38:28
#
#-------------------------------------------------

QT       += charts core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "PXG F1 Telemetry"
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

CONFIG += c++14

SOURCES += \
    CheckUpdatesDialog.cpp \
    FileDownloader.cpp \
    TTGhostsTracker.cpp \
    TelemetryChartView.cpp \
        main.cpp \
        F1Telemetry.cpp \
    F1Listener.cpp \
    UdpSpecification.cpp \
    Tracker.cpp \
    DriverTracker.cpp \
    Lap.cpp \
    TrackingWidget.cpp \
    Logger.cpp \
    Stint.cpp \
    Tyres.cpp \
    Ers.cpp \
    TelemetryData.cpp \
    TelemetryDataTableModel.cpp \
    CompareTelemetryWidget.cpp \
    CompareLapsWidget.cpp \
    CompareStintsWidget.cpp \
    AboutDialog.cpp

HEADERS += \
    CheckUpdatesDialog.h \
        F1Telemetry.h \
    F1Listener.h \
    FileDownloader.h \
    TTGhostsTracker.h \
    TelemetryChartView.h \
    UdpSpecification.h \
    Tracker.h \
    DriverTracker.h \
    Lap.h \
    TrackingWidget.h \
    Logger.h \
    Stint.h \
    Tyres.h \
    Ers.h \
    TelemetryData.h \
    TelemetryDataTableModel.h \
    CompareTelemetryWidget.h \
    CompareLapsWidget.h \
    CompareStintsWidget.h \
    AboutDialog.h

FORMS += \
    CheckUpdatesDialog.ui \
        F1Telemetry.ui \
    TrackingWidget.ui \
    CompareTelemetryWidget.ui \
    AboutDialog.ui


RESOURCES += \
    F1Telemetry.qrc

ICON = ../Ressources/F1Telemetry.icns


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
