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
    ~QCoapClientPrivate();

    CoapMessageMap requestMap;
    QCoapProtocol* protocol;
    QList<QCoapConnection*> connections;
    QThread* workerThread;

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
