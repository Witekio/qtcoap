#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    uri(QUrl()),
    proxyUri(QUrl()),
    operation(EmptyCoapOperation),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    uri(other.uri),
    proxyUri(other.proxyUri),
    operation(other.operation),
    observe(other.observe)
{
}

QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type, const QUrl& proxyUrl) :
    QCoapMessage(*new QCoapRequestPrivate)
{
    setUrl(url);
    setProxyUrl(proxyUrl);
    setType(type);
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // to generate message ids and tokens
}

QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other)
{
}

QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->uri;
}

QUrl QCoapRequest::proxyUrl() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->proxyUri;
}

QCoapOperation QCoapRequest::operation() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->operation;
}

bool QCoapRequest::observe() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->observe;
}

void QCoapRequest::setUrl(const QUrl& url)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->uri == url)
        return;

    d->uri = url;
}

void QCoapRequest::setProxyUrl(const QUrl& proxyUrl)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->proxyUri == proxyUrl)
        return;

    d->proxyUri = proxyUrl;
}

void QCoapRequest::setOperation(QCoapOperation operation)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->operation == operation)
        return;

    d->operation = operation;
}

void QCoapRequest::setObserve(bool observe)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->observe == observe)
        return;

    d->observe = observe;
}

QCoapRequest& QCoapRequest::operator=(const QCoapRequest& other)
{
    d_ptr = other.d_ptr;
    return *this;
}

bool QCoapRequest::operator<(const QCoapRequest& other) const
{
    return (d_ptr->token < other.token());
}

QT_END_NAMESPACE
