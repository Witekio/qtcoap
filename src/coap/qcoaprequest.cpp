#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    url(QUrl()),
    connection(new QCoapConnection),
    protocol(new QCoapProtocol),
    reply(new QCoapReply),
    operation(EMPTY),
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
    delete protocol;
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
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    //qDebug() << "QCoapRequest : sendRequest()";
    //QThread *thread = new QThread();

    // TODO : protocol->sendRequest(this) when thread start
    /*connect(thread, SIGNAL(started()), this, SLOT(_q_startToSend()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(this, SIGNAL(finished(QCoapRequest*)), thread, SLOT(quit()));
    //connect(reply(), SIGNAL(nextBlockAsked(uint)), this, SLOT(_q_getNextBlock(uint))); // Block2
    //connect(reply(), SIGNAL(acknowledgmentAsked(quint16)), this, SLOT(_q_sendAck(quint16))); // CON replies
    //connect(this, SIGNAL(replied()), this, SLOT(_q_startToSend())); // Equivalent of finished for Block2 and observe
    //connect(connection(), SIGNAL(readyRead()), this, SLOT(_q_readReply()));*/

    //d_ptr->connection->moveToThread(thread);
    //d_ptr->protocol->moveToThread(thread);

    //thread->start();
    QObject::connect(d->protocol, SIGNAL(lastBlockReceived(const QCoapInternalReply&)),
                     d->reply, SLOT(updateWithInternalReply(const QCoapInternalReply&)));

    d->protocol->sendRequest(QCoapInternalRequest::fromQCoapRequest(*this), d->connection);
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
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->reply;
}

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

void QCoapRequest::setReply(QCoapReply* reply)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->reply == reply)
        return;

    d->reply = reply;
}

void QCoapRequest::setConnection(QCoapConnection* connection)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->connection == connection)
        return;

    d->connection = connection;
}

void QCoapRequest::setProtocol(QCoapProtocol* protocol)
{
    QCoapRequestPrivate* d = static_cast<QCoapRequestPrivate*>(d_ptr);
    if (d->protocol == protocol)
        return;

    d->protocol = protocol;
}

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

QT_END_NAMESPACE
