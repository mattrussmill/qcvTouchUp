#-------------------------------------------------
#
# Project created by QtCreator 2017-11-11T18:37:42
#
#-------------------------------------------------

QT       += core gui quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qcvTouchUp

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler).
DEFINES += QT_DEPRECATED_WARNINGS

# The following line disables all the APIs deprecated before Qt 5.6.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x056000

# The following line enables the OpenCV functionality in the ImageWidget custom class
DEFINES += QT_OPENCV


# These files are the QT source code and resources for qcvTouchUp
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imagewidget.cpp \
    quickmenu.cpp \
    histogramwidget.cpp \
    histogramwindow.cpp \
    imageworker.cpp \
    adjustmenu.cpp \
    mousewheeleatereventfilter.cpp \
    bufferwrappersqcv.cpp \
    filtermenu.cpp \
    temperaturemenu.cpp \
    transformmenu.cpp

HEADERS += \
        mainwindow.h \
    imagewidget.h \
    quickmenu.h \
    histogramwidget.h \
    histogramwindow.h \
    imageworker.h \
    adjustmenu.h \
    mousewheeleatereventfilter.h \
    bufferwrappersqcv.h \
    filtermenu.h \
    temperaturemenu.h \
    transformmenu.h

FORMS += \
        mainwindow.ui \
    histogramwindow.ui \
    adjustmenu.ui \
    filtermenu.ui \
    temperaturemenu.ui \
    transformmenu.ui

RESOURCES += \
    resources.qrc

DISTFILES +=

#makespec used to help determine qmake configuration for the project
message($$QMAKESPEC)

#configures the make environment (to be used as a template) for Windows build environment
win32{
    message(Windows)

    # The following code links the 3.3.1 OpenCV libraries to qcvTouchUp in windows using
    # OPENCV3_SDK_DIR as the PATH variable for the opencv build directories.
    INCLUDEPATH += $$(OPENCV3_SDK_DIR)/include  #come back later and find out which of these libs are not needed

    # Configured for MinGW 5.3.0 for 32-bit compilation debug
    # and release builds.
    contains(QT_ARCH, i386):{
        CONFIG(debug, debug|release){
            LIBS += -L$$(OPENCV3_SDK_DIR)/x86/mingw53/bin \
                -lopencv_core331d \
                -lopencv_highgui331d \
                -lopencv_imgcodecs331d \
                -lopencv_imgproc331d \
                -lopencv_photo331d
        }
        CONFIG(release, debug|release){
            LIBS += -L$$(OPENCV3_SDK_DIR)/x86/mingw53/bin \
                -lopencv_core331 \
                -lopencv_highgui331 \
                -lopencv_imgcodecs331 \
                -lopencv_imgproc331 \
                -lopencv_photo331
        }
    }

    # Configured for MSVC2017 15.0 for 64-bit compilation release
    # build. 64-bit debug build is not configured in Qt Creator.
    contains(QT_ARCH, x86_64):{
        CONFIG(release, debug|release){
            LIBS += -L$$(OPENCV3_SDK_DIR)/x86_64/vc15/lib/ \
                -lopencv_core331 \
                -lopencv_highgui331 \
                -lopencv_imgcodecs331 \
                -lopencv_imgproc331 \
                -lopencv_photo331
        } else:message(x86_64 debug NOT configured!)
    }
}

#configures the make environment (to be used as a template) for Windows build environment
linux-g++ {
    message(Linux)

    # The following code links the 3.3.1 OpenCV libraries to qcvTouchUp in linux using
    # the default opencv build directories from the opencv linux install tutorial.
    INCLUDEPATH += /usr/local/include/opencv

    LIBS += -L/usr/local/lib/ \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -lopencv_imgproc \
        -lopencv_photo
}

