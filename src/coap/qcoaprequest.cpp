#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

QCoapRequestPrivate::QCoapRequestPrivate() :
    QCoapMessagePrivate(),
    connection_p(new QCoapConnection()),
    reply_p(new QCoapReply()),
    operation_p(QCoapRequest::GET)
{
    qDebug() << "QCoapRequestPrivate::QCoapRequestPrivate()";
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
    delete connection_p;
}

QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent) :
    QCoapMessage(* new QCoapRequestPrivate, parent)
{
    //Q_UNUSED(url);
    qDebug() << "QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent)" << url;
    Q_D(QCoapRequest);
    d->url_p = url;
    //url_p = url;
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
    return d_func()->url_p;
    //return url_p;
}

void QCoapRequest::setUrl(const QUrl& url)
{
    Q_D(QCoapRequest);
    d->url_p = url;
    //url_p = url;
}

QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    return d_func()->operation_p;
}
void QCoapRequest::setOperation(QCoapRequestOperation operation)
{
    Q_D(QCoapRequest);
    d->operation_p = operation;
}
