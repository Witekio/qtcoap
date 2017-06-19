TEMPLATE = lib

QT += core-private network

HEADERS += \
    qcoapclient.h \
    qcoapconnection.h \
    qcoapmessage.h \
    qcoapoption.h \
    qcoapreply.h \
    qcoaprequest.h \
    qcoapmessage_p.h \
    qcoapreply_p.h \
    qcoaprequest_p.h \
    qcoapoption_p.h \
    qcoapconnection_p.h \
    qcoapclient_p.h \
    qcoapresource.h \
    qcoapresource_p.h \
    qcoapprotocol.h \
    qcoapprotocol_p.h

SOURCES += \
    qcoapclient.cpp \
    qcoapconnection.cpp \
    qcoapmessage.cpp \
    qcoapoption.cpp \
    qcoapreply.cpp \
    qcoaprequest.cpp \
    qcoapresource.cpp \
    qcoapprotocol.cpp
