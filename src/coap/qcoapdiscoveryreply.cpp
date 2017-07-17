#include "qcoapdiscoveryreply.h"
#include "qcoapdiscoveryreply_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCoapDiscoveryReply
    \brief The QCoapDiscoveryReply is a QCoapReply that stores also a
    list of QCoapResources.

    This class is used for discovery requests.

    \sa QCoapClient, QCoapRequest, QCoapReply
*/

/*!
    Constructs a new QCoapDiscoveryReply and sets \a parent as parent object.
*/
QCoapDiscoveryReply::QCoapDiscoveryReply(QObject *parent) :
    QCoapReply (* new QCoapDiscoveryReplyPrivate, parent)
{

}

/*!
    Returns the list of resources.
*/
QList<QCoapResource> QCoapDiscoveryReply::resourceList() const
{
    return d_func()->resources;
}

/*!
    \internal

    Update the QCoapDiscoveryReply object, its message and list of resources
    with data of the internal reply
    \a internalReply.
*/
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
