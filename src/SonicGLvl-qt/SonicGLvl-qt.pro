#-------------------------------------------------
#
# Project created by QtCreator 2015-06-12T12:17:07
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SonicGLvl-qt
TEMPLATE = app

Release:DESTDIR = ../../bin
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = ../../bin/debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui


SOURCES += main.cpp\
        EditorWindow.cpp \
    OgreViewportWidget.cpp \
    OgreSystem.cpp \
    EditorViewerGrid.cpp \
    EditorViewportsWidget.cpp \
    EditorViewport.cpp

HEADERS  += EditorWindow.h \
    OgreViewportWidget.h \
    OgreSystem.h \
    OgreCommon.h \
    QtCommon.h \
    EditorViewerGrid.h \
    EditorViewportsWidget.h \
    EditorViewport.h

FORMS    += EditorWindow.ui

INCLUDEPATH += ../../depends/ogre/include \
               ../../depends/ogre/include/OGRE \
               ../../depends/ogre/boost \
               ../../depends/pthreads/include \
               ../../depends/allegro/include \
               ../../depends/fbxsdk/include \
               ../LibGens \
               ../LibGens-externals


LIBS += -L../../depends/ogre/lib/Release -lOgreMain

QMAKE_LIBDIR += ../../depends/ogre/boost/lib



