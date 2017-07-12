#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    url(QUrl()),
    proxyUrl(QUrl()),
    //connection(new QCoapConnection),
    operation(EmptyOperation),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    url(other.url),
    proxyUrl(other.proxyUrl),
    //connection(other.connection),
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

// TODO : move parseUri into internalRequest : addUriOptions
// (and remove from constructor) + take care of proxy
void QCoapRequest::parseUri()
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);

    // Convert host to QCoapOption if it is not an ip address
    // TODO : ipv6
    QRegExp ipv4Regex("^([0-9]{1,3}.){3}([0-9]{1,3})$");
    QString host = d->url.host();
    if (!ipv4Regex.exactMatch(host)) {
        addOption(QCoapOption::UriHostOption, host.toUtf8());
    }

    // Convert port into QCoapOption if it is not the default port
    int port = d->url.port();
    if (port > 0 && port != 5683){
        addOption(QCoapOption::UriPortOption, QByteArray::number(port, 10));
    }

    // Convert path into QCoapOptions
    QString path = d->url.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::UriPathOption, pathPart.toUtf8());
    }

    // Convert query into QCoapOptions
    QString query = d->url.query();
    QStringList listQuery = query.split("&");
    for (QString query : listQuery) {
        if (!query.isEmpty())
            addOption(QCoapOption::UriQueryOption, query.toUtf8());
    }

    //d->connection->setHost(d->url.host());
    //d->connection->setPort(d->url.port(5683));
}

QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->url;
}

QUrl QCoapRequest::proxyUrl() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->proxyUrl;
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

void QCoapRequest::setProxyUrl(const QUrl& proxyUrl)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->proxyUrl == proxyUrl)
        return;

    d->proxyUrl = proxyUrl;
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
