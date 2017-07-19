#ifndef QCOAPRESOURCE_P_H
#define QCOAPRESOURCE_P_H

#include "qcoapresource.h"

QT_BEGIN_NAMESPACE

class QCoapResourcePrivate
{
public:
    QCoapResourcePrivate();
    ~QCoapResourcePrivate();

    // The comments obs, rt, etc..., are the names of the fields in
    // the protocol.

    QString path;
    QString title;
    bool observable;        // obs
    QString resourceType;   // rt
    QString interface;      // if
    int maximumSize;        // sz
    uint contentFormat;     // ct
};

QT_END_NAMESPACE

#endif // QCOAPRESOURCE_P_H
