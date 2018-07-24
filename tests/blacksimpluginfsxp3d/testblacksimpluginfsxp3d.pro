load(common_pre)

QT       += core testlib dbus network gui widgets

TARGET   = testsimpluginfsxp3d
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += blackmisc blackcore blackconfig blackgui
CONFIG   += simulatorfsxcommon simulatorfscommon simulatorplugincommon fsuipc simconnect
CONFIG   += testcase
CONFIG   += no_testcase_installs

DEPENDPATH += . $$SourceRoot/src
INCLUDEPATH += . $$SourceRoot/src
HEADERS += *.h
SOURCES += *.cpp
DESTDIR = $$DestRoot/bin

# Ignore linker warning about missing pdb files from Simconnect
msvc: QMAKE_LFLAGS *= /ignore:4099

load(common_post)