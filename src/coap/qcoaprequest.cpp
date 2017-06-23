#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    url(QUrl()),
    connection(new QCoapConnection),
    //protocol(new QCoapProtocol),
    //reply(new QCoapReply),
    operation(EMPTY),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    url(other.url),
    connection(other.connection),
    //protocol(other.protocol),
    //reply(other.reply),
    operation(other.operation),
    state(other.state),
    observe(other.observe)

{
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
    delete connection;
    //delete protocol;
}

QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type) :
    QCoapMessage(*new QCoapRequestPrivate)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);

    d->url = url;
    parseUri();
    setState(QCoapRequest::WAITING);
    setType(type);
    qsrand(QTime::currentTime().msec()); // to generate message ids and tokens
}

QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other)
{
}

QCoapRequest::~QCoapRequest()
{
}

void QCoapRequest::sendRequest() const
{
    // TODO : move to the client
    /*QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);

    QThread *thread = new QThread();

   d->protocol->prepareToSendRequest(QCoapInternalRequest::fromQCoapRequest(*this),
                                      d->connection);

    QObject::connect(thread, SIGNAL(started()), d->protocol, SLOT(startToSend()));
    QObject::connect(d->reply, SIGNAL(finished()), thread, SLOT(quit()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    QObject::connect(d->protocol, SIGNAL(lastBlockReceived(const QCoapInternalReply&)),
                     d->reply, SLOT(updateWithInternalReply(const QCoapInternalReply&)));

    d->reply->moveToThread(thread); // To use the parameter in the signals/slots
    d->connection->moveToThread(thread);
    d->connection->socket()->moveToThread(thread); // The socket is not directly a child of connection
    d->protocol->moveToThread(thread);

    thread->start();*/
}

void QCoapRequest::parseUri()
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    // Convert path into QCoapOptions
    QString path = d->url.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::URIPATH, pathPart.toUtf8());
    }

    d->connection->setHost(d->url.host());
    d->connection->setPort(d->url.port(5683));
}

/*void QCoapRequest::setRequestForAck(quint16 messageId, const QByteArray& payload)
{
    setType(ACKNOWLEDGMENT);
    setOperation(EMPTY);
    setMessageId(messageId);
    // NOTE : verify if we change the token, if we use the reply token or if we have no token
    // (and for the reset message too)
    // setToken(token);
    setPayload(payload);
    removeAllOptions();
}

void QCoapRequest::setRequestForReset(quint16 messageId)
{
    setType(RESET);
    setOperation(EMPTY);
    setMessageId(messageId);
    //setToken(QByteArray(""));
    setPayload(QByteArray(""));
    removeAllOptions();
}*/

/*QCoapReply* QCoapRequest::reply() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->reply;
}*/

QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->url;
}

QCoapConnection* QCoapRequest::connection() const
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->connection;
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
    if (d->url == url)
        return;

    d->url = url;
}

/*void QCoapRequest::setReply(QCoapReply* reply)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->reply == reply)
        return;

    d->reply = reply;
}*/

void QCoapRequest::setConnection(QCoapConnection* connection)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->connection == connection)
        return;

    d->connection = connection;
}

/*void QCoapRequest::setProtocol(QCoapProtocol* protocol)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->protocol == protocol)
        return;

    d->protocol = protocol;
}*/

void QCoapRequest::setOperation(QCoapOperation operation)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->operation == operation)
        return;

    d->operation = operation;
}

void QCoapRequest::setState(QCoapRequestState state)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->state == state)
        return;

    d->state = state;
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
