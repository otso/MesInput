TEMPLATE = app
TARGET = mesinput
DEPENDPATH += .
INCLUDEPATH += .

CONFIG += link_pkgconfig debug meegotouch
QT += network script xml
HEADERS += mesfetcher.h \
    mainpage.h \
    layoutlistpage.h
SOURCES += mesfetcher.cpp \
           main.cpp \
    mainpage.cpp \
    layoutlistpage.cpp

desktop_icon.path = /usr/share/icons/hicolor/scalable/apps
desktop_icon.files = icon-l-mesinput.png

desktop_entry.path = /usr/share/applications
desktop_entry.files = mesinput.desktop

data_files.files = data/*
data_files.path = /usr/share/mesinput/
data_files.CONFIG += no_check_exist

target.path = /usr/bin/

INSTALLS += target \
    data_files \
    desktop_entry \
    desktop_icon
