QT += testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    ../../../src/coap/qcoapclient.h \
    ../../../src/coap/qcoapconnection.h \
    ../../../src/coap/qcoapmessage.h \
    ../../../src/coap/qcoapoption.h \
    ../../../src/coap/qcoapreply.h \
    ../../../src/coap/qcoaprequest.h

SOURCES += \
    ../../../src/coap/qcoapclient.cpp \
    ../../../src/coap/qcoapconnection.cpp \
    ../../../src/coap/qcoapmessage.cpp \
    ../../../src/coap/qcoapoption.cpp \
    ../../../src/coap/qcoapreply.cpp \
    ../../../src/coap/qcoaprequest.cpp

SOURCES += \ 
    tst_qcoapconnection.cpp

INCLUDEPATH += ../../../src/coap/
