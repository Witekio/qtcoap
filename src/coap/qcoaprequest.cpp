#include "qcoaprequest.h"

QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent) :
    QObject(parent)
{
    Q_UNUSED(url);
    // TODO
}

QByteArray QCoapRequest::toPdu()
{
    // TODO
    return QByteArray("");
}

void QCoapRequest::sendRequest()
{
    // TODO
}

void QCoapRequest::readReply()
{
    // TODO
}

qint16 QCoapRequest::generateMessageId()
{
    // TODO
    return 0;
}

qint64 QCoapRequest::generateToken()
{
    // TODO
    return 0;
}

QByteArray QCoapRequest::rawReply()
{
    // TODO
    return QByteArray("");
}

QUrl QCoapRequest::url() const
{
    // TODO
    return QUrl("invalid-url-waiting-function-to-be-coded");
}

void QCoapRequest::setUrl(const QUrl& url)
{
    Q_UNUSED(url);
}

QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    // TODO
    return QCoapRequestOperation::OTHER;
}
void QCoapRequest::setOperation(QCoapRequestOperation operation)
{
    Q_UNUSED(operation);
}
