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

QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
    qRegisterMetaType<QCoapInternalRequest*>();
}

void QCoapProtocol::sendRequest(QCoapReply* reply, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    // connect with QueuedConnection type to secure from deleting the reply (reply destructor emit the signal)
    connect(reply, SIGNAL(aborted(QCoapReply*)), this, SLOT(onAbortedRequest(QCoapReply*)), Qt::QueuedConnection);
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)), reply, SLOT(connectionError(QAbstractSocket::SocketError)));

    if(!reply)
        return;

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
    copyInternalRequest->setTimeout(d->ackTimeout);
    connect(copyInternalRequest, SIGNAL(timeout(QCoapInternalRequest*)),
            this, SLOT(resendRequest(QCoapInternalRequest*)));

    // Invoke to change current thread
    QMetaObject::invokeMethod(this, "sendRequest", Q_ARG(QCoapInternalRequest*, copyInternalRequest));
}

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

void QCoapProtocolPrivate::messageReceived(const QByteArray& frameReply)
{
    //qDebug() << "QCoapProtocol::messageReceived() - " << frameReply;
    frameQueue.enqueue(frameReply);
    if (frameQueue.size() == 1)
        handleFrame();
}

void QCoapProtocolPrivate::handleFrame()
{
    handleFrame(frameQueue.head());
}

void QCoapProtocolPrivate::resendRequest(QCoapInternalRequest* request)
{
    // In case of retransmission, check if it is not the last try
    if (internalReplies.contains(request)) {
        if (request->retransmissionCounter() < maxRetransmit) {
            sendRequest(request);
        } else {
            internalReplies[request].userReply->abortRequest();
            internalReplies[request].userReply->setError(QCoapReply::TimeOutCoapError);
        }
    }
}

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

QCoapInternalRequest* QCoapProtocolPrivate::findInternalRequestByToken(const QByteArray& token)
{
    /*for (QCoapInternalRequest request : internalReplies.keys()) {
        if (request.message()->token() == token)
             return request;
    }*/

    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message().token() == token)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

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

void QCoapProtocolPrivate::onLastBlock(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::onLastBlock()";
    Q_Q(QCoapProtocol);

    QList<QCoapInternalReply*> replies = internalReplies[request].replies;
    QCoapReply* userReply = internalReplies[request].userReply;

    if (replies.isEmpty() || !userReply)
        return;

    QCoapInternalReply* finalReply(replies.last());
    if (finalReply->message().type() == QCoapMessage::AcknowledgmentMessage
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

void QCoapProtocolPrivate::onNextBlock(QCoapInternalRequest* request,
                                       uint currentBlockNumber,
                                       uint blockSize)
{
    qDebug() << "QCoapProtocol::onNextBlock()";
    request->setRequestToAskBlock(currentBlockNumber+1, blockSize);
    sendRequest(request);
}

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

void QCoapProtocol::cancelObserve(QCoapReply* reply)
{
    if (!reply)
        return;

    Q_D(QCoapProtocol);
    // TODO : move this into another function
    QCoapInternalRequest* copyRequest = nullptr;
    for (InternalMessageMap::Iterator it = d->internalReplies.begin(); it != d->internalReplies.end(); ++it) {
        if (it.value().userReply == reply) {
            copyRequest = const_cast<QCoapInternalRequest*>(it.key());
            break;
        }
    }
    copyRequest->setCancelObserve(true);
}

QByteArray QCoapProtocolPrivate::encode(QCoapInternalRequest* request)
{
    return request->toQByteArray();
}

QCoapInternalReply* QCoapProtocolPrivate::decode(const QByteArray& message)
{
    Q_Q(QCoapProtocol);
    return new QCoapInternalReply(QCoapInternalReply::fromQByteArray(message), q);
    //return QCoapInternalReply::fromQByteArray(message);
}

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

bool QCoapProtocolPrivate::containsToken(const QByteArray& token)
{
    for (QCoapInternalRequest* request : internalReplies.keys()) {
        if (request->message().token() == token)
            return true;
    }

    return false;
}

bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (QCoapInternalRequest* request : internalReplies.keys()) {
        if (request->message().messageId() == id)
            return true;
    }

    return false;
}

uint QCoapProtocol::ackTimeout() const
{
    return d_func()->ackTimeout;
}

double QCoapProtocol::ackRandomFactor() const
{
    return d_func()->ackRandomFactor;
}

uint QCoapProtocol::maxRetransmit() const
{
    return d_func()->maxRetransmit;
}

quint16 QCoapProtocol::blockSize() const
{
    return d_func()->blockSize;
}

void QCoapProtocol::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapProtocol);
    d->ackTimeout = ackTimeout;
}

void QCoapProtocol::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapProtocol);
    d->ackRandomFactor = ackRandomFactor;
}

void QCoapProtocol::setMaxRetransmit(uint maxRetransmit)
{
    Q_D(QCoapProtocol);
    d->maxRetransmit = maxRetransmit;
}

void QCoapProtocol::setBlockSize(quint16 blockSize)
{
    // A size of 0 mean that the server choose the blocks size
    Q_D(QCoapProtocol);
    d->blockSize = blockSize;
}

QT_END_NAMESPACE

#include "moc_qcoapprotocol.cpp"
