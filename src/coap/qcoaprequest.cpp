#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

QCoapRequestPrivate::QCoapRequestPrivate() :
    //QCoapMessagePrivate(),
    url_p(QUrl()),
    connection_p(new QCoapConnection()),
    reply_p(new QCoapReply()),
    operation_p(QCoapRequest::GET)
{
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
    delete connection_p;
}

QCoapRequest::QCoapRequest(const QUrl& url, QObject* parent) :
    QCoapMessage(* new QCoapRequestPrivate, parent)
{
    Q_D(QCoapRequest);
    d->url_p = url;
    parseUri();
    qsrand(QTime::currentTime().msec());
}

/*QCoapRequest::QCoapRequest(const QCoapRequest &other)
{
    Q_D(QCoapRequest);
    d->url_p = other.url();
    d->connection_p = other.connection();
    d->reply_p = other.reply();
    d->operation_p = other.operation();
}*/

QByteArray QCoapRequest::toPdu()
{
    // TODO : finish request to pdu method
    Q_D(QCoapRequest);
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(0x01) << 30)      // Coap version
            | (quint32(d->type_p) << 28)            // Message type
            | (quint32(d->tokenLength_p) << 24)     // Token Length
            | (quint32(d->operation_p) << 16)       // operation type
            | (quint32(d->messageId_p));            // message ID

    pdu.append(quint8(coapHeader >> 24));
    pdu.append(quint8((coapHeader >> 16) & 0xFF));
    pdu.append(quint8((coapHeader >> 8) & 0xFF));
    pdu.append(quint8(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d->token_p);

    // Insert Options
    if (!d->options_p.isEmpty()) {
        qSort(d->options_p.begin(), d->options_p.end(),
              [](const QCoapOption* a, const QCoapOption* b) -> bool {
            return (a->name() < b->name());
        });
        quint8 lastOptionNumber = 0;
        for (QCoapOption* option : d->options_p) {
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
        pdu.append(d->payload_p);
    }

    return pdu;
}

void QCoapRequest::sendRequest()
{
    // TODO
}

void QCoapRequest::readReply()
{
    // TODO
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
    // TODO : autotest parseUri ?
    Q_D(QCoapRequest);
    QString path = d->url_p.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::URIPATH, pathPart.toUtf8());
    }
}

QCoapReply* QCoapRequest::reply() const
{
    return d_func()->reply_p;
}

QUrl QCoapRequest::url() const
{
    return d_func()->url_p;
}

QCoapConnection* QCoapRequest::connection() const
{
    return d_func()->connection_p;
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
