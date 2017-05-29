#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

QCoapRequestPrivate::QCoapRequestPrivate() :
    QCoapMessagePrivate(),
    //url_p(QUrl()),
    connection_p(nullptr),
    reply_p(nullptr),
    operation_p(QCoapRequest::GET)
{
}

QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent) :
    QCoapMessage(parent)
{
    //Q_UNUSED(url);
    Q_D(QCoapRequest);
    d->url_p = url;
    d->operation_p = QCoapRequest::GET;
    //d_func()->url_p = url;
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

QCoapReply* QCoapRequest::reply()
{
    return d_func()->reply_p;
}

QUrl QCoapRequest::url() const
{
    return /*d_func()->url_p;//*/QUrl("invalid-url-waiting-function-to-be-coded");
}

void QCoapRequest::setUrl(const QUrl& url)
{
    Q_UNUSED(url);
}

QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    return d_func()->operation_p;
}
void QCoapRequest::setOperation(QCoapRequestOperation operation)
{
    Q_UNUSED(operation);
}
