QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    binmap.cpp \
    gsfunc.cpp \
    imgfunc.cpp \
    iofunc.cpp \
    jfunc.cpp \
    main.cpp \
    mainwindow.cpp \
    mathfunc.cpp \
    qtfunc.cpp \
    qtpaint.cpp \
    sqlfunc.cpp \
    statscan.cpp \
    switchboard.cpp \
    winfunc.cpp \
    zipfunc.cpp

HEADERS += \
    binmap.h \
    gsfunc.h \
    imgfunc.h \
    iofunc.h \
    jfunc.cpp \
    mainwindow.h \
    mathfunc.h \
    qtfunc.h \
    qtpaint.h \
    sqlfunc.h \
    sqlite3.h \
    statscan.h \
    stb_image.h \
    stb_image_write.h \
    stb_truetype.h \
    switchboard.h \
    ui_mainwindow.h \
    winfunc.h \
    zipfunc.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
