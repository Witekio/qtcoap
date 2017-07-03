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
    void setBlockSize(quint16 blockSize);

    static QList<QCoapResource> resourcesFromCoreLinkList(const QByteArray& data);

public slots:
    void messageReceived(const QByteArray& frameReply);
    void abortRequest(QCoapReply* reply);

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
