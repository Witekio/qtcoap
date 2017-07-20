#ifndef QCOAPCLIENT_P_H
#define QCOAPCLIENT_P_H

#include "qcoapclient.h"
#include <private/qobject_p.h>
#include <QThread>
#include <QPointer>

QT_BEGIN_NAMESPACE

typedef QMap<QCoapRequest, QPointer<QCoapReply>> CoapMessageMap;

class QCoapClientPrivate : public QObjectPrivate
{
public:
    QCoapClientPrivate();
    ~QCoapClientPrivate();

    CoapMessageMap requestMap;
    QCoapProtocol* protocol;
    QCoapConnection* connection;
    QThread* workerThread;

    QCoapReply* sendRequest(const QCoapRequest& request);
    QCoapDiscoveryReply* sendDiscovery(const QCoapRequest& request);

    void setConnection(QCoapConnection* newConnection);

    Q_DECLARE_PUBLIC(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_P_H
