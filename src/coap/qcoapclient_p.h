#ifndef QCOAPCLIENT_P_H
#define QCOAPCLIENT_P_H

#include "qcoapclient.h"
#include <private/qobject_p.h>
#include <QThread>

QT_BEGIN_NAMESPACE

typedef QMap<QCoapRequest, QCoapReply*> CoapMessageMap;

class QCoapClientPrivate : public QObjectPrivate
{
public:
    QCoapClientPrivate();

    CoapMessageMap requestMap;
    QCoapProtocol* protocol;
    QList<QCoapConnection*> connections;
    QList<QCoapResource*> resources;
    QThread* workerThread;

    int findRequestByToken(QByteArray token);
    bool containsToken(QByteArray token);
    bool containsMessageId(quint16 id);

    void _q_requestFinished(QCoapRequest* request);

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
