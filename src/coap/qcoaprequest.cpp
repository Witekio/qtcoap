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

/*!
    \class QCoapRequest
    \brief The QCoapRequest class holds a coap request. This request
    can be sent with QCoapClient.

    The QCoapRequest contains data needed to make coap frames that can be
    sent to the url it holds.

    \sa QCoapClient, QCoapReply, QCoapDiscoveryReply
*/

/*!
    Constructs a QCoapRequest object with the target \a url,
    the proxy url \a proxyUrl and the \a type of the message.
*/
QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type, const QUrl& proxyUrl) :
    QCoapMessage(*new QCoapRequestPrivate)
{
    setUrl(url);
    setProxyUrl(proxyUrl);
    setType(type);
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // to generate message ids and tokens
}

/*!
    Constructs a copy of the \a other QCoapRequest
*/
QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other)
{
}

/*!
    Returns the target uri of the request.

    \sa setUrl()
*/
QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->uri;
}

/*!
    Returns the proxy uri of the request.

    \sa setProxyUrl()
*/
QUrl QCoapRequest::proxyUrl() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->proxyUri;
}

/*!
    Returns the operation of the request.

    \sa setOperation()
*/
QCoapOperation QCoapRequest::operation() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->operation;
}

/*!
    Returns true if the request is an observe request.

    \sa setObserve()
*/
bool QCoapRequest::observe() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->observe;
}

/*!
    Sets the target uri of the request.

    \sa url()
*/
void QCoapRequest::setUrl(const QUrl& url)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->uri == url)
        return;

    d->uri = url;
}

/*!
    Sets the proxy uri of the request.

    \sa proxyUrl()
*/
void QCoapRequest::setProxyUrl(const QUrl& proxyUrl)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->proxyUri == proxyUrl)
        return;

    d->proxyUri = proxyUrl;
}

/*!
    Sets the operation of the request.

    \sa operation()
*/
void QCoapRequest::setOperation(QCoapOperation operation)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->operation == operation)
        return;

    d->operation = operation;
}

/*!
    Sets the observe to true to make an observe request.

    \sa observe()
*/
void QCoapRequest::setObserve(bool observe)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->observe == observe)
        return;

    d->observe = observe;
}

/*!
    Creates a copy of \a other.
*/
QCoapRequest& QCoapRequest::operator=(const QCoapRequest& other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns true if the message id of the current request is lower than
    the message id of \a other.
*/
bool QCoapRequest::operator<(const QCoapRequest& other) const
{
    return (d_ptr->messageId < other.messageId());
}

QT_END_NAMESPACE
