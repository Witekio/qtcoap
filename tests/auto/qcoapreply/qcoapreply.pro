QT += testlib core-private network core
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
    ../../../src/coap/qcoaprequest.h \
    ../../../src/coap/qcoapmessage_p.h \
    ../../../src/coap/qcoapreply_p.h \
    ../../../src/coap/qcoaprequest_p.h \
    ../../../src/coap/qcoapoption_p.h \
    ../../../src/coap/qcoapconnection_p.h \
    ../../../src/coap/qcoapclient_p.h \
    ../../../src/coap/qcoapresource.h \
    ../../../src/coap/qcoapresource_p.h

SOURCES += \
    ../../../src/coap/qcoapclient.cpp \
    ../../../src/coap/qcoapconnection.cpp \
    ../../../src/coap/qcoapmessage.cpp \
    ../../../src/coap/qcoapoption.cpp \
    ../../../src/coap/qcoapreply.cpp \
    ../../../src/coap/qcoaprequest.cpp \
    ../../../src/coap/qcoapresource.cpp

SOURCES += \ 
    tst_qcoapreply.cpp

INCLUDEPATH += ../../../src/coap/

