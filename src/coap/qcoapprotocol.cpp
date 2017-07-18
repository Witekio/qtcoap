#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QT_BEGIN_NAMESPACE

QCoapProtocolPrivate::QCoapProtocolPrivate() :
    blockSize(0),
    ackTimeout(2000),
    ackRandomFactor(1.5),
    maxRetransmit(4)
{
}

/*!
    \class QCoapProtocol
    \brief The QCoapProtocol class handle the logical part of the CoAP protocol.

    \reentrant

    The QCoapProtocol is used by the QCoapClient class to handle the logical
    part of the protocol. It can encode requests and decode replies. It also
    handle what to do when a message is received and the retransmission of
    lost messages.

    \sa QCoapClient
*/

/*!
    Constructs a new QCoapProtocol and sets \a parent as the parent object.
*/
QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
    qRegisterMetaType<QCoapInternalRequest*>();
}

/*!
    Creates and sets up a new QCoapInternalRequest related to the request
    associated to the \a reply. The request will then be sent to the server
    using the given \a connection.
*/
void QCoapProtocol::sendRequest(QCoapReply* reply, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    // connect with QueuedConnection type to secure from deleting the reply (reply destructor emit the signal)
    connect(reply, SIGNAL(aborted(QCoapReply*)), this, SLOT(onAbortedRequest(QCoapReply*)), Qt::QueuedConnection);
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), reply, SLOT(connectionError(QAbstractSocket::SocketError)));

    if(!reply)
        return;

    // Generate unique token and message id
    QCoapInternalRequest* copyInternalRequest = new QCoapInternalRequest(reply->request());
    copyInternalRequest->moveToThread(this->thread()); // put "this" as parent does not work
    qDebug() << copyInternalRequest->thread() << " " << this->thread();
    if (copyInternalRequest->message().messageId() == 0) {
        do {
            copyInternalRequest->generateMessageId();
        } while (d->containsMessageId(copyInternalRequest->message().messageId()));
    }
    if (copyInternalRequest->message().token().isEmpty()) {
        do {
            copyInternalRequest->generateToken();
        } while (d->containsToken(copyInternalRequest->message().token()));
    }

    copyInternalRequest->setConnection(connection);

    // If this request does not already exist we add it to the map
    if (!d->findInternalRequestByToken(copyInternalRequest->message().token())) {
            InternalMessagePair pair = { reply, QList<QCoapInternalReply*>() };
            d->internalReplies[copyInternalRequest] = pair;
    }

    // If the user specified a size for blockwise request/replies
    //qDebug() << "payload length : " << copyInternalRequest->message()->payload().length()
    //         << " - blockSize : " << d->blockSize;
    if (d->blockSize > 0) {
        copyInternalRequest->setRequestToAskBlock(0, d->blockSize);
        if (copyInternalRequest->message().payload().length() > d->blockSize)
            copyInternalRequest->setRequestToSendBlock(0, d->blockSize);
    }

    reply->setIsRunning(true);
    if (copyInternalRequest->message().type() == QCoapMessage::ConfirmableCoapMessage) {
        copyInternalRequest->setTimeout(d->ackTimeout);
        connect(copyInternalRequest, SIGNAL(timeout(QCoapInternalRequest*)),
                this, SLOT(resendRequest(QCoapInternalRequest*)));
    }

    // Invoke to change current thread
    QMetaObject::invokeMethod(this, "sendRequest", Q_ARG(QCoapInternalRequest*, copyInternalRequest));
}

/*!
    Encode and sends the given \a request to the server.
*/
void QCoapProtocolPrivate::sendRequest(QCoapInternalRequest* request)
{
    /*qDebug() << request->thread()
             << request->timer()->thread()
             << QThread::currentThread()
             << q_ptr->thread();*/
    request->beginTransmission();
    QByteArray requestFrame = encode(request);
    QUrl uri = request->targetUri();
    request->connection()->sendRequest(requestFrame, uri.host(), uri.port());
}

/*!
    Handle what to do when a new message is received.

    Here, it puts the frame \a frameReply into a queue and run the function
    that handle the frames if it is the first.
*/
void QCoapProtocolPrivate::messageReceived(const QByteArray& frameReply)
{
    //qDebug() << "QCoapProtocol::messageReceived() - " << frameReply;
    frameQueue.enqueue(frameReply);
    if (frameQueue.size() == 1)
        handleFrame();
}

/*!
    Take the first frame of the queue (without removing it)
    and handle it.
*/
void QCoapProtocolPrivate::handleFrame()
{
    handleFrame(frameQueue.head());
}

/*!
    This slot is used to send again the given \a request after a timeout or
    abort the request and transfer a timeout error to the reply.
*/
void QCoapProtocolPrivate::resendRequest(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocolPrivate::resendRequest()";
    // In case of retransmission, check if it is not the last try
    if (request->message().type() == QCoapMessage::ConfirmableCoapMessage && internalReplies.contains(request)) {
        qDebug() << "CONFIRMABLE and REQUEST EXIST";
        if (request->retransmissionCounter() < maxRetransmit) {
            sendRequest(request);
        } else {
            internalReplies[request].userReply->abortRequest();
            internalReplies[request].userReply->setError(QCoapReply::TimeOutCoapError);
        }
    }
}

/*!
    Handle the given \a frame and take the next.
*/
void QCoapProtocolPrivate::handleFrame(const QByteArray& frame)
{
    qDebug() << "QCoapProtocol::handleFrame()";
    QCoapInternalReply* internalReply = decode(frame);
    QCoapInternalRequest* request = nullptr;
    QCoapMessage internalReplyMessage = internalReply->message();

    /*qDebug() << "MID : " << internalReply.messageId()
             << " - Token : " << internalReply.token();*/

    if (!internalReplyMessage.token().isEmpty())
        request = findInternalRequestByToken(internalReplyMessage.token());

    if (!request) {
        request = findInternalRequestByMessageId(internalReplyMessage.messageId());
        if (!request) {
            qDebug() << "No request found (handleFrame)";
            return;
        }
    }

    request->stopTransmission();
    internalReplies[request].replies.push_back(internalReply);

    // Reply when the server ask an ACK
    if (request->cancelObserve()) {
        // Remove option to ensure that it will stop
        request->removeOptionByName(QCoapOption::ObserveCoapOption);
        sendReset(request);
    } else if (internalReplyMessage.type() == QCoapMessage::ConfirmableCoapMessage) {
        sendAcknowledgment(request);
    }

    // Check if it is a blockwise request
    int nextBlockWanted = internalReply->wantNextBlock();

    // Ask/Send next block or process the final reply
    if (internalReply->hasNextBlock()) {
        onNextBlock(request, internalReply->currentBlockNumber(), internalReply->blockSize());
    } else if (nextBlockWanted >= 0) {
        request->setRequestToSendBlock(static_cast<uint>(nextBlockWanted), blockSize);
        sendRequest(request);
    } else
        onLastBlock(request);

    // Take the next frame if needed
    frameQueue.dequeue();
    //qDebug() << frameQueue.length();
    if (!frameQueue.isEmpty())
        handleFrame();
}

/*!
    Find an internal request containing the given \a token.
*/
QCoapInternalRequest* QCoapProtocolPrivate::findInternalRequestByToken(const QByteArray& token)
{
    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message().token() == token)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    Find an internal request matching the given \a reply.
*/
QCoapInternalRequest* QCoapProtocolPrivate::findInternalRequestByReply(QCoapReply* reply)
{
    QCoapInternalRequest* copyRequest = nullptr;
    for (InternalMessageMap::Iterator it = internalReplies.begin(); it != internalReplies.end(); ++it) {
        if (it.value().userReply == reply) {
            copyRequest = const_cast<QCoapInternalRequest*>(it.key());
            break;
        }
    }

    return copyRequest;
}

/*!
    Find an internal request containing the message id \a messageId.
*/
QCoapInternalRequest* QCoapProtocolPrivate::findInternalRequestByMessageId(quint16 messageId)
{
    /*for (QCoapInternalRequest request : internalReplies.keys()) {
        if (request.message()->messageId() == messageId)
             return &request;
    }*/
    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message().messageId() == messageId)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    Handle what to do when we received the last block of a reply.

    Here it merge all blocks, remove the request from the map,
    update the associated QCoapReply and emit a finished signal.
*/
void QCoapProtocolPrivate::onLastBlock(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::onLastBlock()";
    Q_Q(QCoapProtocol);

    QList<QCoapInternalReply*> replies = internalReplies[request].replies;
    QCoapReply* userReply = internalReplies[request].userReply;

    if (replies.isEmpty() || !userReply)
        return;

    QCoapInternalReply* finalReply(replies.last());
    if (finalReply->message().type() == QCoapMessage::AcknowledgmentCoapMessage
            && finalReply->statusCode() == InvalidCoapCode)
        return;

    // If multiple blocks : append data from all blocks to the final reply
    if (replies.size() > 1) {
        qStableSort(std::begin(replies), std::end(replies),
              [](QCoapInternalReply* a, QCoapInternalReply* b) -> bool {
            return (a->currentBlockNumber() < b->currentBlockNumber());
        });

        QByteArray finalPayload;
        int lastBlockNumber = -1;
        for (QCoapInternalReply* reply : replies) {
            int currentBlock = static_cast<int>(reply->currentBlockNumber());
            QByteArray replyPayload = reply->message().payload();
            if (replyPayload.isEmpty() || currentBlock <= lastBlockNumber)
                continue;

            finalPayload.append(replyPayload);
            lastBlockNumber = static_cast<int>(reply->currentBlockNumber());
        }

        finalReply->message().setPayload(finalPayload);
    }

    // Remove the request or the replies
    if (!userReply->request().observe() || request->cancelObserve())
        internalReplies.remove(request);
    else
        internalReplies[request].replies.clear();

    if (userReply && !userReply->isAborted()) {
        userReply->updateFromInternalReply(*finalReply);
        emit q->finished(userReply);
    }
}

/*!
    Handle what to do when we received a new block that is not the last.

    Here it set the given internal \a request to ask the block number
    that follow \a currentBlockNumber with a size of \a blockSize
    and send this new request.
*/
void QCoapProtocolPrivate::onNextBlock(QCoapInternalRequest* request,
                                       uint currentBlockNumber,
                                       uint blockSize)
{
    qDebug() << "QCoapProtocol::onNextBlock()";
    request->setRequestToAskBlock(currentBlockNumber+1, blockSize);
    sendRequest(request);
}

/*!
    Create a new internal request with the same uri and connection than
    \a request, set it up to make it an acknowledgment message and send it.
*/
void QCoapProtocolPrivate::sendAcknowledgment(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    QCoapInternalRequest ackRequest;
    QCoapInternalReply* internalReply = internalReplies[request].replies.last();

    ackRequest.setTargetUri(request->targetUri());
    ackRequest.initForAcknowledgment(internalReply->message().messageId(),
                                     internalReply->message().token());
    ackRequest.setConnection(request->connection());
    sendRequest(&ackRequest);
}

/*!
    Create a new internal request with the same uri and connection than
    \a request, set it up to make it a reset message and send it.
*/
void QCoapProtocolPrivate::sendReset(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::sendReset()";
    QCoapInternalRequest resetRequest;
    QCoapInternalReply* internalReply = internalReplies[request].replies.last();

    resetRequest.setTargetUri(request->targetUri());
    resetRequest.initForReset(internalReply->message().messageId());
    resetRequest.setConnection(request->connection());
    sendRequest(&resetRequest);
}

/*!
    Handle what to do when the user want to stop to observe a resource.

    Here it just find the internal request associated to the \a reply and
    set it to ask to cancel the observe request.
*/
void QCoapProtocol::cancelObserve(QCoapReply* reply)
{
    if (!reply)
        return;

    Q_D(QCoapProtocol);
    QCoapInternalRequest* copyRequest = d->findInternalRequestByReply(reply);
    if (copyRequest)
        copyRequest->setCancelObserve(true);
}

/*!
    Encode the QCoapInternalRequest object \a request to a QByteArray frame.
*/
QByteArray QCoapProtocolPrivate::encode(QCoapInternalRequest* request)
{
    return request->toQByteArray();
}

/*!
    Decode the QByteArray \a message to a QCoapInternalReply object.
*/
QCoapInternalReply* QCoapProtocolPrivate::decode(const QByteArray& message)
{
    Q_Q(QCoapProtocol);
    return new QCoapInternalReply(QCoapInternalReply::fromQByteArray(message), q);
    //return QCoapInternalReply::fromQByteArray(message);
}

/*!
    Handle what to do when the request corresponding to the given
    \a reply has been aborted.

    Here it stops the transmission of the request and remove it from the map.
*/
void QCoapProtocolPrivate::onAbortedRequest(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::onAbortedRequest()";
    QCoapInternalRequest* request = findInternalRequestByReply(reply);
    if (request) {
        qDebug() << "REMOVE REQUEST";
        request->stopTransmission();
        internalReplies.remove(request);
    }
}

/*!
    Decode the QByteArray \a data to a list QCoapResource objects.
    The QByteArray \a data is a frame returned by a discovery request.
*/
QList<QCoapResource> QCoapProtocol::resourcesFromCoreLinkList(const QByteArray& data)
{
    QList<QCoapResource> resourceList;

    QList<QByteArray> links = data.split(',');
    for (QByteArray link : links)
    {
        QCoapResource resource;
        QList<QByteArray> parameterList = link.split(';');
        for (QByteArray parameter : parameterList)
        {
            int length = parameter.length();
            if (parameter.startsWith('<'))
                resource.setPath(QString(parameter).mid(1, length-2));
            else if (parameter.startsWith("title="))
                resource.setTitle(QString(parameter).right(length-6).remove("\""));
            else if (parameter.startsWith("rt="))
                resource.setResourceType(QString(parameter).right(length-3).remove("\""));
            else if (parameter.startsWith("if="))
                resource.setInterface(QString(parameter).right(length-3).remove("\""));
            else if (parameter.startsWith("sz="))
                resource.setMaximumSize(QString(parameter).right(length-3).remove("\"").toInt());
            else if (parameter.startsWith("ct="))
                resource.setContentFormat(QString(parameter).right(length-3).remove("\"").toUInt());
            else if (parameter == "obs")
                resource.setObservable(true);
        }

        if (!resource.path().isEmpty())
            resourceList.push_back(resource);
    }

    return resourceList;
}

/*!
    Returns true if a request have a token equal to \a token.
*/
bool QCoapProtocolPrivate::containsToken(const QByteArray& token)
{
    for (QCoapInternalRequest* request : internalReplies.keys()) {
        if (request->message().token() == token)
            return true;
    }

    return false;
}

/*!
    Returns true if a request have a message id equal to \a id.
*/
bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (QCoapInternalRequest* request : internalReplies.keys()) {
        if (request->message().messageId() == id)
            return true;
    }

    return false;
}

/*!
    Returns the ACK_TIMEOUT value.

    \sa setAckTimeout()
*/
uint QCoapProtocol::ackTimeout() const
{
    return d_func()->ackTimeout;
}

/*!
    Returns the ACK_RANDOM_FACTOR value.

    \sa setAckRandomFactor()
*/
double QCoapProtocol::ackRandomFactor() const
{
    return d_func()->ackRandomFactor;
}

/*!
    Returns the MAX_RETRANSMIT value.

    \sa setMaxRetransmit()
*/
uint QCoapProtocol::maxRetransmit() const
{
    return d_func()->maxRetransmit;
}

/*!
    Returns the max block size wanted.

    \sa setBlockSize()
*/
quint16 QCoapProtocol::blockSize() const
{
    return d_func()->blockSize;
}

/*!
    Sets the ACK_TIMEOUT value.

    \sa ackTimeout()
*/
void QCoapProtocol::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapProtocol);
    d->ackTimeout = ackTimeout;
}

/*!
    Sets the ACK_RANDOM_FACTOR value.

    \sa ackRandomFactor()
*/
void QCoapProtocol::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapProtocol);
    d->ackRandomFactor = ackRandomFactor;
}

/*!
    Sets the MAX_RETRANSMIT value.

    \sa maxRetransmit()
*/
void QCoapProtocol::setMaxRetransmit(uint maxRetransmit)
{
    Q_D(QCoapProtocol);
    d->maxRetransmit = maxRetransmit;
}

/*!
    Sets the max block size wanted.

    \sa blockSize()
*/
void QCoapProtocol::setBlockSize(quint16 blockSize)
{
    // A size of 0 mean that the server choose the blocks size
    Q_D(QCoapProtocol);
    d->blockSize = blockSize;
}

QT_END_NAMESPACE

#include "moc_qcoapprotocol.cpp"
