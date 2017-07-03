#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    url(QUrl()),
    //connection(new QCoapConnection),
    operation(EmptyOperation),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    url(other.url),
    //connection(other.connection),
    operation(other.operation),
    observe(other.observe)

{
}

QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type) :
    QCoapMessage(*new QCoapRequestPrivate)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);

    d->url = url;
    parseUri();
    setType(type);
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // to generate message ids and tokens
}

QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other)
{
}

void QCoapRequest::parseUri()
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    // Convert path into QCoapOptions
    QString path = d->url.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::UriPathOption, pathPart.toUtf8());
    }

    //d->connection->setHost(d->url.host());
    //d->connection->setPort(d->url.port(5683));
}

QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->url;
}

/*QCoapConnection* QCoapRequest::connection() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->connection;
}*/

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
    if (d->url == url)
        return;

    d->url = url;
}

/*void QCoapRequest::setConnection(QCoapConnection* connection)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->connection == connection)
        return;

    d->connection = connection;
}*/

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
