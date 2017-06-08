#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

QCoapRequestPrivate::QCoapRequestPrivate() :
    //QCoapMessagePrivate(),
    url(QUrl()),
    connection(new QCoapConnection()),
    reply(new QCoapReply()),
    operation(QCoapRequest::GET)
{
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
    delete connection;
}

QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent) :
    QCoapMessage(* new QCoapRequestPrivate, parent)
{
    Q_D(QCoapRequest);
    d->url = url;
    parseUri();
    setState(QCoapRequest::CREATED);
    qsrand(QTime::currentTime().msec()); // to generate message ids and tokens
}

/*QCoapRequest::QCoapRequest(const QCoapRequest &other)
{
    Q_D(QCoapRequest);
    d->url = other.url();
    d->connection = other.connection();
    d->reply = other.reply();
    d->operation = other.operation();
}*/

QByteArray QCoapRequest::toPdu()
{
    Q_D(QCoapRequest);
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(d->version) << 30)      // Coap version
            | (quint32(d->type) << 28)            // Message type
            | (quint32(d->tokenLength) << 24)     // Token Length
            | (quint32(d->operation) << 16)       // Operation type
            | (quint32(d->messageId));            // Message ID

    pdu.append(quint8(coapHeader >> 24));
    pdu.append(quint8((coapHeader >> 16) & 0xFF));
    pdu.append(quint8((coapHeader >> 8) & 0xFF));
    pdu.append(quint8(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d->token);

    // Insert Options
    if (!d->options.isEmpty()) {
        // Sort options by ascending order
        qSort(d->options.begin(), d->options.end(),
              [](const QCoapOption* a, const QCoapOption* b) -> bool {
            return (a->name() < b->name());
        });
        quint8 lastOptionNumber = 0;
        for (QCoapOption* option : d->options) {
            quint8 optionPdu =
                    ((quint8(option->name()) - lastOptionNumber) << 4)  // Option Delta
                    | (option->length() & 0x0F);                        // Option Length
            pdu.append(optionPdu);
            pdu.append(option->value());                                // Option Value
            lastOptionNumber = option->name();
        }
    }

    // Insert Payload
    if (!payload().isEmpty()) {
        pdu.append(char(0xFF));
        pdu.append(d->payload);
    }

    return pdu;
}

void QCoapRequest::sendRequest()
{
    //qDebug() << "QCoapRequest : sendRequest()";
    QThread *thread = new QThread();

    connect(thread, SIGNAL(started()), this, SLOT(startToSend()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(connection(), SIGNAL(readyRead()), this, SLOT(readReply()));

    connection()->moveToThread(thread);

    thread->start();
    /*connect(connection(), SIGNAL(readyRead()), this, SLOT(readReply()));
    startToSend();*/
}

void QCoapRequest::readReply()
{
    Q_D(QCoapRequest);

    if (d->state != REPLIED) {
        setState(REPLIED);
        reply()->fromPdu(connection()->readReply());
        emit finished(this);
    }
}

quint16 QCoapRequest::generateMessageId()
{
    // TODO : improve generation of message id ?
    quint16 id = qrand() % 65536;
    return id;
}

QByteArray QCoapRequest::generateToken()
{
    // TODO : improve generation of token ?
    QByteArray token("");
    quint8 length = (qrand() % 7) + 1;
    token.resize(length);

    quint8 *tokenData = (quint8 *)token.data();
    for (int i = 0; i < token.size(); ++i)
        tokenData[i] = qrand() % 256;

    return token;
}

void QCoapRequest::parseUri() {
    Q_D(QCoapRequest);
    QString path = d->url.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::URIPATH, pathPart.toUtf8());
    }

    d->connection->setHost(d->url.host());
    d->connection->setPort(d->url.port(5683));
}

QCoapReply* QCoapRequest::reply() const
{
    return d_func()->reply;
}

QUrl QCoapRequest::url() const
{
    return d_func()->url;
}

QCoapConnection* QCoapRequest::connection() const
{
    return d_func()->connection;
}

void QCoapRequest::setUrl(const QUrl& url)
{
    Q_D(QCoapRequest);
    if (d->url == url)
        return;

    d->url = url;
}

void QCoapRequest::setReply(QCoapReply* reply)
{
    Q_D(QCoapRequest);
    if (d->reply == reply)
        return;

    d->reply = reply;
}

void QCoapRequest::setConnection(QCoapConnection* connection)
{
    Q_D(QCoapRequest);
    if (d->connection == connection)
        return;

    d->connection = connection;
}

void QCoapRequest::startToSend()
{
    qDebug() << "QCoapRequest : startToSend() - " << this->toPdu().toHex();
    connection()->sendRequest(this->toPdu());
    setState(QCoapRequest::SENT);
}

QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    return d_func()->operation;
}

void QCoapRequest::setOperation(QCoapRequestOperation operation)
{
    Q_D(QCoapRequest);
    if (d->operation == operation)
        return;

    d->operation = operation;
}

void QCoapRequest::setState(QCoapRequestState state)
{
    Q_D(QCoapRequest);
    if (d->state == state)
        return;

    d->state = state;
}
