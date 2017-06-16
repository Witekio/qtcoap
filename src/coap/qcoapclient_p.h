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
    QList<QCoapResource*> resources;
    int findRequestByToken(QByteArray token);
    bool containsToken(QByteArray token);
    bool containsMessageId(quint16 id);
    void _q_requestFinished(QCoapRequest* request);

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
