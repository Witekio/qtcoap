#include "qcoapdiscoveryreply.h"
#include "qcoapdiscoveryreply_p.h"

QT_BEGIN_NAMESPACE

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

    QCoapMessage internalReplyMessage = internalReply.message();

    d_func()->message.setPayload(internalReplyMessage.payload());
    d_func()->message.setType(internalReplyMessage.type());
    d_func()->message.setVersion(internalReplyMessage.version());
    d_func()->status = internalReply.statusCode();

    d_func()->isFinished = true;
    d_func()->isRunning = false;

    d->resources = QCoapProtocol::resourcesFromCoreLinkList(internalReplyMessage.payload());

    emit finished();
}

QT_END_NAMESPACE
