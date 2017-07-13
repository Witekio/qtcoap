#ifndef QCOAPPROTOCOL_H
#define QCOAPPROTOCOL_H

#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoapconnection.h"
#include "qcoapreply.h"
#include "qcoapresource.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QCoapProtocolPrivate;
class QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = 0);

    void sendRequest(QCoapReply* reply, QCoapConnection* connection);
    void cancelObserve(QCoapReply* reply);

    uint ackTimeout() const;
    double ackRandomFactor() const;
    uint maxRetransmit() const;
    quint16 blockSize() const;
    void setAckTimeout(uint ackTimeout);
    void setAckRandomFactor(double ackRandomFactor);
    void setMaxRetransmit(uint maxRetransmit);
    void setBlockSize(quint16 blockSize);

    static QList<QCoapResource> resourcesFromCoreLinkList(const QByteArray& data);

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)
    Q_PRIVATE_SLOT(d_func(), void resendRequest(QCoapInternalRequest*))
    Q_PRIVATE_SLOT(d_func(), void sendRequest(QCoapInternalRequest*))
    Q_PRIVATE_SLOT(d_func(), void messageReceived(const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void onAbortedRequest(QCoapReply* reply))
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
