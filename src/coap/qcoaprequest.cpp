#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    QCoapMessagePrivate (),
    url(QUrl()),
    connection(new QCoapConnection),
    protocol(new QCoapProtocol),
    reply(new QCoapReply()),
    operation(QCoapRequest::GET),
    blockAsked(0),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    url(other.url),
    connection(other.connection),
    protocol(other.protocol),
    reply(other.reply),
    operation(other.operation),
    state(other.state),
    blockAsked(other.blockAsked),
    observe(other.observe)

{
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
    delete connection;
}

QCoapRequest::QCoapRequest(const QUrl& url, QCoapMessageType type) :
    QCoapMessage ()
{
    d_ptr->url = url;
    parseUri();
    setState(QCoapRequest::WAITING);
    setType(type);
    qsrand(QTime::currentTime().msec()); // to generate message ids and tokens
}

QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other),
    d_ptr(other.d_ptr)
{
}

QCoapRequest::~QCoapRequest()
{
}

QByteArray QCoapRequest::toPdu()
{
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(d_ptr->version) << 30)    // Coap version
            | (quint32(d_ptr->type) << 28)                  // Message type
            | (quint32(d_ptr->tokenLength) << 24)           // Token Length
            | (quint32(d_ptr->operation) << 16)             // Operation type
            | (quint32(d_ptr->messageId));                  // Message ID

    pdu.append(static_cast<quint8>(coapHeader >> 24));
    pdu.append(static_cast<quint8>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<quint8>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<quint8>(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d_ptr->token);

    // Insert Options
    if (!d_ptr->options.isEmpty()) {
        // Sort options by ascending order
        qSort(d_ptr->options.begin(), d_ptr->options.end(),
              [](const QCoapOption& a, const QCoapOption& b) -> bool {
            return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (QCoapOption option : d_ptr->options) {
            quint8 optionPdu;

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            quint16 optionLength = option.length();
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
            pdu.append(option.value());            // Option Value

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!payload().isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d_ptr->payload);
    }

    return pdu;
}

void QCoapRequest::sendRequest()
{
    //qDebug() << "QCoapRequest : sendRequest()";
    QThread *thread = new QThread();

    // TODO : protocol->sendRequest(this) when thread start
    /*connect(thread, SIGNAL(started()), this, SLOT(_q_startToSend()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(finished(QCoapRequest*)), thread, SLOT(quit()));
    //connect(reply(), SIGNAL(nextBlockAsked(uint)), this, SLOT(_q_getNextBlock(uint))); // Block2
    //connect(reply(), SIGNAL(acknowledgmentAsked(quint16)), this, SLOT(_q_sendAck(quint16))); // CON replies
    //connect(this, SIGNAL(replied()), this, SLOT(_q_startToSend())); // Equivalent of finished for Block2 and observe
    //connect(connection(), SIGNAL(readyRead()), this, SLOT(_q_readReply()));*/

    d_ptr->connection->moveToThread(thread);
    d_ptr->protocol->moveToThread(thread);

    thread->start();
}

void QCoapRequest::readReply()
{
    //Q_D(QCoapRequest);

    //d->_q_readReply();
    return;
}

/*void QCoapRequestPrivate::_q_readReply()
{
    Q_Q(QCoapRequest);

    q->setState(QCoapRequest::REPLIED);
    //if (state != QCoapRequest::REPLYCOMPLETE) {
        QByteArray replyFromSocket = connection->readReply();
        reply->fromPdu(replyFromSocket);

        // If it is the last block
        if (!reply->hasNextBlock()) {
            q->setState(QCoapRequest::REPLYCOMPLETE);
            if (observe) {
                qDebug() << "EMIT NOTIFIED";
                emit q->notified(reply->readData());
            } else {
                qDebug() << "EMIT FINISHED";
                emit q->finished(q);
            }
        } else {
            qDebug() << "WAITING";
            q->setState(QCoapRequest::WAITING);
            emit q->replied();
        }
    //}
}*/

/*void QCoapRequestPrivate::_q_getNextBlock(uint blockNumber)
{
    Q_Q(QCoapRequest);

    // NOTE : if it does not enter there, it does not get next block
    // but without this, sometimes, it ask the same block twice
    //if (blockNumber > blockAsked) {
        blockAsked = blockNumber;
        // Set the BLOCK2 option to get the new block
        q->addOptionToAskBlock(blockNumber);

        //emit q->replied();
    //} // TODO: find a better way to do this (in case of block already asked)
    //else {
    //    if (reply->hasNextBlock())
    //        _q_getNextBlock(blockNumber+1);
    //}

    qDebug() << " BLOCK ASKED : " << blockNumber;
}*/

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

void QCoapRequest::parseUri()
{
    // Convert path into QCoapOptions
    QString path = d_ptr->url.path();
    QStringList listPath = path.split("/");
    for (QString pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::URIPATH, pathPart.toUtf8());
    }

    d_ptr->connection->setHost(d_ptr->url.host());
    d_ptr->connection->setPort(d_ptr->url.port(5683));
}

void QCoapRequest::addOptionToAskBlock(uint blockNumber)
{
    // Set the BLOCK2 option to get the new block
    quint32 block2Data = (blockNumber << 4) | 2;
    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(block2Data >> 16);
    if (block2Data > 0xFF)
        block2Value.append(block2Data >> 8 & 0xFF);
    block2Value.append(block2Data & 0xFF);

    removeOptionByName(QCoapOption::BLOCK2);
    addOption(QCoapOption::BLOCK2, block2Value);

    setToken(generateToken());
    setMessageId(d_ptr->messageId+1);
}

void QCoapRequest::setRequestForAck(quint16 messageId, const QByteArray& payload)
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
}

/*void QCoapRequestPrivate::_q_startToSend()
{
    Q_Q(QCoapRequest);

    //qDebug() << "QCoapRequest : startToSend() - " << q->toPdu().toHex();
    if (state == QCoapRequest::WAITING) {
        q->setState(QCoapRequest::SENT);
        connection->sendRequest(q->toPdu());
    }
}*/

/*void QCoapRequestPrivate::_q_sendAck(quint16 messageId)
{
    Q_Q(QCoapRequest);

    qDebug() << "SEND ACK SLOT";
    q->setRequestForAck(messageId);
    emit q->replied();
}*/

QCoapReply* QCoapRequest::reply() const
{
    return d_ptr->reply;
}

QUrl QCoapRequest::url() const
{
    return d_ptr->url;
}

QCoapConnection* QCoapRequest::connection() const
{
    return d_ptr->connection;
}

QCoapRequest::QCoapRequestOperation QCoapRequest::operation() const
{
    return d_ptr->operation;
}

bool QCoapRequest::observe() const
{
    return d_ptr->observe;
}

void QCoapRequest::setUrl(const QUrl& url)
{
    if (d_ptr->url == url)
        return;

    d_ptr->url = url;
}

void QCoapRequest::setReply(QCoapReply* reply)
{
    if (d_ptr->reply == reply)
        return;

    d_ptr->reply = reply;
}

void QCoapRequest::setConnection(QCoapConnection* connection)
{
    if (d_ptr->connection == connection)
        return;

    d_ptr->connection = connection;
}

void QCoapRequest::setProtocol(QCoapProtocol* protocol)
{
    if (d_ptr->protocol == protocol)
        return;

    d_ptr->protocol = protocol;
}

void QCoapRequest::setOperation(QCoapRequestOperation operation)
{
    if (d_ptr->operation == operation)
        return;

    d_ptr->operation = operation;
}

void QCoapRequest::setState(QCoapRequestState state)
{
    if (d_ptr->state == state)
        return;

    d_ptr->state = state;
}

void QCoapRequest::setObserve(bool observe)
{
    if (d_ptr->observe == observe)
        return;

    d_ptr->observe = observe;
}

QCoapRequest& QCoapRequest::operator=(const QCoapRequest& other)
{
    d_ptr = other.d_ptr;
    return *this;
}

QT_END_NAMESPACE
