#ifndef QCOAPCLIENT_P_H
#define QCOAPCLIENT_P_H

#include "qcoapclient.h"
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QCoapClientPrivate : public QObjectPrivate
{
public:
    QCoapClientPrivate();

    QList<QCoapRequest*> requests;

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
