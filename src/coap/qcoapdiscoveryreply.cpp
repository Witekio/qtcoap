#include "qcoapdiscoveryreply.h"
#include "qcoapdiscoveryreply_p.h"

QCoapDiscoveryReply::QCoapDiscoveryReply(QObject *parent) :
    QCoapReply (* new QCoapDiscoveryReplyPrivate, parent)
{

}

QList<QCoapResource> QCoapDiscoveryReply::resourceList() const
{
    return d_func()->resources;
}

void QCoapDiscoveryReply::updateFromInternalReply(const QCoapInternalReply& internalReply)
{
    Q_D(QCoapDiscoveryReply);

    d_func()->message.setPayload(internalReply.payload());
    d_func()->message.setType(internalReply.type());
    d_func()->message.setVersion(internalReply.version());
    d_func()->status = internalReply.statusCode();

    d_func()->isFinished = true;
    d_func()->isRunning = false;

    d->resources = QCoapProtocol::resourcesFromCoreLinkList(internalReply.payload());

    emit finished();
}
