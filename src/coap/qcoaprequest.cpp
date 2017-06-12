#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
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

QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type, QObject* parent) :
    QCoapMessage(* new QCoapRequestPrivate, parent)
{
    Q_D(QCoapRequest);
    d->url = url;
    parseUri();
    setState(QCoapRequest::CREATED);
    setType(type);
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

    pdu.append(static_cast<quint8>(coapHeader >> 24));
    pdu.append(static_cast<quint8>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<quint8>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<quint8>(coapHeader & 0xFF));

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
            quint8 optionPdu;

            quint16 optionDelta = static_cast<quint16>(option->name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            quint16 optionLength = option->length();
            bool isOptionLengthExtended = false;
            quint8 optionLengthExtended = 0;

            // Delta value > 12 : special values
            if (optionDelta > 268) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 269);
                optionDelta = 14;
                isOptionDeltaExtended = true;
            } else if (optionDelta > 12) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 13);
                optionDelta = 13;
                isOptionDeltaExtended = true;
            }

            // Length > 12 : special values
            if (optionLength > 268) {
                optionLengthExtended = static_cast<quint8>(optionLength - 269);
                optionLength = 14;
                isOptionLengthExtended = true;
            } else if (optionLength > 12) {
                optionLengthExtended = static_cast<quint8>(optionLength - 13);
                optionLength = 13;
                isOptionLengthExtended = true;
            }

            optionPdu = (static_cast<quint8>(optionDelta) << 4)          // Option Delta
                        | (static_cast<quint8>(optionLength) & 0x0F);    // Option Length
            pdu.append(optionPdu);
            if (isOptionDeltaExtended)
                pdu.append(optionDeltaExtended);    // Option Delta Extended
            if (isOptionLengthExtended)
                pdu.append(optionLengthExtended);   // Option Length Extended
            pdu.append(option->value());            // Option Value

            lastOptionNumber = option->name();
        }
    }

    // Insert Payload
    if (!payload().isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d->payload);
    }

    return pdu;
}

void QCoapRequest::sendRequest()
{
    //qDebug() << "QCoapRequest : sendRequest()";
    QThread *thread = new QThread();

    connect(thread, SIGNAL(started()), this, SLOT(_q_startToSend()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(connection(), SIGNAL(readyRead()), this, SLOT(_q_readReply()));
    // TODO : resend a request for blockwised transfer
    //connect (this, SIGNAL(repliedBlockwise()), this, SLOT(_q_getNextBlock()));
    connection()->moveToThread(thread);

    thread->start();
}

void QCoapRequest::readReply()
{
    Q_D(QCoapRequest);

    d->_q_readReply();
}

void QCoapRequestPrivate::_q_readReply()
{
    Q_Q(QCoapRequest);

    if (state != QCoapRequest::REPLIED) {
        reply->fromPdu(connection->readReply());
        if (reply->hasNextBlock()) {
            quint16 block2Data = (static_cast<quint16>(reply->currentBlockNumber()+1) << 4) | 2;
            QByteArray block2Value = QByteArray();
            block2Value.append(block2Data >> 8);
            block2Value.append(block2Data & 0xFF);
            q->addOption(QCoapOption::BLOCK2, block2Value);
            emit q->repliedBlockwise();
        } else {
            q->setState(QCoapRequest::REPLIED);
            emit q->finished(q);
        }
    }
}

quint16 QCoapRequest::generateMessageId()
{
    quint16 id = qrand() % 65536;
    return id;
}

QByteArray QCoapRequest::generateToken()
{
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


void QCoapRequestPrivate::_q_startToSend()
{
    Q_Q(QCoapRequest);

    qDebug() << "QCoapRequest : startToSend() - " << q->toPdu().toHex();
    connection->sendRequest(q->toPdu());
    q->setState(QCoapRequest::SENT);
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


QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    return d_func()->operation;
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

QT_END_NAMESPACE

#include "moc_qcoaprequest.cpp"
