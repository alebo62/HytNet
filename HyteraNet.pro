QT -= gui
QT += network
CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        checksum.cpp \
        gps.cpp \
        main.cpp \
        msg_init.cpp \
        route.cpp \
        rrs.cpp \
        tcp.cpp \
        tcp_srv.cpp \
        timers.cpp \
        txt.cpp \
        udp_rad.cpp \
        udp_rcp.cpp \
        udp_srv.cpp

HEADERS += \
    enums.h \
    messages.h \
    tcp.h \
    variables.h

target.path = /home/pi
INSTALLS += target


