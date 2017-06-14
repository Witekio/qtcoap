#ifndef QCOAPRESOURCE_P_H
#define QCOAPRESOURCE_P_H

#include <qcoapresource.h>

QT_BEGIN_NAMESPACE

class QCoapResourcePrivate
{
public:
    QCoapResourcePrivate();
    ~QCoapResourcePrivate();

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
