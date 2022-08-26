QT       += concurrent core gui network webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20 object_parallel_to_source

include(lib/innertube-qt/innertube-qt.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ui/settingsform.cpp

HEADERS += \
    browsehelper.hpp \
    mainwindow.h \
    settingsstore.hpp \
    ui/clickablelabel.h \
    ui/homevideorenderer.h \
    ui/settingsform.h

FORMS += \
    mainwindow.ui \
    ui/settingsform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
