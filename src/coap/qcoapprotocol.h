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

    void handleFrame(const QByteArray& frame);
    void onLastBlock(QCoapReply* request);
    void onNextBlock(QCoapReply* reply, uint currentBlockNumber);
    void sendAcknowledgment(QCoapReply* reply);
    void sendReset(QCoapReply* reply);
    void cancelObserve(QCoapReply* reply);

    void sendRequest(QCoapReply* reply, QCoapConnection* connection);
    QByteArray encode(const QCoapInternalRequest& request);
    QCoapInternalReply decode(const QByteArray& message);
    static QList<QCoapResource> resourcesFromCoreLinkList(const QByteArray& data);

signals:
    void lastBlockReceived(const QCoapInternalReply&);
    void frameQueueFilled();

public slots:
    void messageReceived(const QByteArray& frameReply);

private:

    Q_DECLARE_PRIVATE(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
