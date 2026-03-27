QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = systeme_presence
TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    enseignantdialog.cpp \
    enseignantwindow.cpp \
    etudiantdialog.cpp \
    main.cpp \
    loginwindow.cpp \
    mainwindow.cpp \
    matieredialog.cpp \
    seancedialog.cpp

HEADERS += \
    enseignantdialog.h \
    enseignantwindow.h \
    etudiantdialog.h \
    loginwindow.h \
    mainwindow.h \
    matieredialog.h \
    seancedialog.h

FORMS += \
    enseignantdialog.ui \
    enseignantwindow.ui \
    etudiantdialog.ui \
    loginwindow.ui \
    mainwindow.ui \
    matieredialog.ui \
    seancedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
