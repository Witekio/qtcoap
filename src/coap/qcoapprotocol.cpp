#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QT_BEGIN_NAMESPACE

QCoapProtocolPrivate::QCoapProtocolPrivate() :
    state(WAITING),
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

/*void QCoapProtocol::sendRequest(QCoapReply* reply, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    connect(reply, SIGNAL(abortRequest(QCoapReply*)), this, SLOT(abortRequest(QCoapReply*)), Qt::QueuedConnection);

    if(!reply)
        return;

    QCoapInternalRequest copyInternalRequest;
    copyInternalRequest = QCoapInternalRequest::fromQCoapRequest(reply->request());

    if (copyInternalRequest.messageId() == 0) {
        do {
            copyInternalRequest.generateMessageId();
        } while (d->containsMessageId(copyInternalRequest.messageId()));
    }
    if (copyInternalRequest.token().isEmpty()) {
        do {
            copyInternalRequest.generateToken();
        } while (d->containsToken(copyInternalRequest.token()));
    }

    copyInternalRequest.setConnection(connection);

    // If this request does not already exist we add it to the map
    if (!d->findReplyByToken(copyInternalRequest.token())) {
        InternalMessagePair pair = { copyInternalRequest, QList<QCoapInternalReply>() };
        d->internalReplies[reply] = pair;
    }

    reply->setIsRunning(true);
    d->sendRequest(copyInternalRequest);
}*/

void QCoapProtocol::sendRequest(QCoapReply* reply, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    // TODO : find a way to secure from deleting the reply
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
    if (!d->findRequestByToken(copyInternalRequest->message().token())) {
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
    // TODO : something like this but we need the request to resend
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
    request->connection()->sendRequest(requestFrame);
}

void QCoapProtocol::messageReceived(const QByteArray& frameReply)
{
    //qDebug() << "QCoapProtocol::messageReceived() - " << frameReply;
    Q_D(QCoapProtocol);

    d->frameQueue.enqueue(frameReply);
    if (d->frameQueue.size() == 1)
        d->handleFrame();
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

/*void QCoapProtocolPrivate::handleFrame(const QByteArray& frame)
{
    //qDebug() << "QCoapProtocol::handleFrame()";
    QCoapInternalReply internalReply = decode(frame);
    QCoapReply* reply = nullptr;
    if (!internalReply.token().isEmpty())
        reply = findReplyByToken(internalReply.token());

    if (!reply) {
        reply = findReplyByMessageId(internalReply.messageId());
        if (!reply) {
            qDebug() << "No reply found (handleFrame)";
            return;
        }
    }

    internalReplies[reply].replies.push_back(internalReply);

    // Reply when the server ask an ACK
    if (internalReplies[reply].request.cancelObserve())
        sendReset(reply);
    else if (internalReply.type() == QCoapMessage::ConfirmableMessage)
        sendAcknowledgment(reply);

    // Ask next block or process the final reply
    if (internalReply.hasNextBlock())
        onNextBlock(reply, internalReply.currentBlockNumber());
    else
        onLastBlock(reply);

    // Take the next frame if needed
    frameQueue.dequeue();
    if (!frameQueue.isEmpty())
        handleFrame();
}*/

void QCoapProtocolPrivate::handleFrame(const QByteArray& frame)
{
    qDebug() << "QCoapProtocol::handleFrame()";
    QCoapInternalReply* internalReply = decode(frame);
    QCoapInternalRequest* request = nullptr;
    QCoapMessage internalReplyMessage = internalReply->message();

    /*qDebug() << "MID : " << internalReply.messageId()
             << " - Token : " << internalReply.token();*/

    if (!internalReplyMessage.token().isEmpty())
        request = findRequestByToken(internalReplyMessage.token());

    if (!request) {
        request = findRequestByMessageId(internalReplyMessage.messageId());
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
        request->removeOptionByName(QCoapOption::ObserveOption);
        sendReset(request);
    } else if (internalReplyMessage.type() == QCoapMessage::ConfirmableMessage)
        sendAcknowledgment(request);

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

/*QCoapReply* QCoapProtocolPrivate::findReplyByToken(const QByteArray& token)
{
    for (QCoapReply* reply : internalReplies.keys()) {
        if (internalReplies[reply].request.token() == token)
             return reply;
    }

    return nullptr;
}*/

QCoapInternalRequest* QCoapProtocolPrivate::findRequestByToken(const QByteArray& token)
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

QCoapInternalRequest* QCoapProtocolPrivate::findInternalRequest(QCoapReply* reply)
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

/*QCoapReply* QCoapProtocolPrivate::findReplyByMessageId(quint16 messageId)
{
    for (QCoapReply* reply : internalReplies.keys()) {
        if (internalReplies[reply].request.messageId() == messageId)
             return reply;
    }

    return nullptr;
}*/

QCoapInternalRequest* QCoapProtocolPrivate::findRequestByMessageId(quint16 messageId)
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

/*void QCoapProtocolPrivate::onLastBlock(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::onLastBlock()";

    if (!reply)
        return;

    QList<QCoapInternalReply> replies = internalReplies[reply].replies;
    //QCoapInternalRequest request = internalReplies[reply].request;
    if (replies.isEmpty())
        return;

    QCoapInternalReply finalReply(replies.last());

    // If multiple blocks : append data from all blocks to the final reply
    if (replies.size() > 1 && !reply->request().observe()) {
        qSort(std::begin(replies), std::end(replies),
              [](const QCoapInternalReply& a, const QCoapInternalReply& b) -> bool {
            return (a.currentBlockNumber() < b.currentBlockNumber());
        });

        QByteArray finalPayload;
        for (const QCoapInternalReply& reply : replies) {
            QByteArray replyPayload = reply.payload();
            if (replyPayload.isEmpty())
                continue;

            finalPayload.append(replyPayload);
        }

        finalReply.setPayload(finalPayload);
    }

    // Remove the reply
    if (!reply->request().observe())
        internalReplies.remove(reply);

    if (reply) {
        qDebug() << "PASS";
        reply->updateFromInternalReply(finalReply);
        qDebug() << "PASS2";
    }

    //emit lastBlockReceived(finalReply);
}*/

void QCoapProtocolPrivate::onLastBlock(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::onLastBlock()";

    QList<QCoapInternalReply*> replies = internalReplies[request].replies;
    QCoapReply* userReply = internalReplies[request].userReply;

    if (replies.isEmpty() || !userReply)
        return;

    QCoapInternalReply* finalReply(replies.last());
    if (finalReply->message().type() == QCoapMessage::AcknowledgmentMessage
            && finalReply->statusCode() == InvalidCode)
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

    if (userReply)
        userReply->updateFromInternalReply(*finalReply);
}

/*void QCoapProtocolPrivate::onNextBlock(QCoapReply* reply, uint currentBlockNumber)
{
    if (!reply)
        return;

    //qDebug() << "QCoapProtocol::onNextBlock()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;

    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    sendRequest(copyRequest);
}*/

void QCoapProtocolPrivate::onNextBlock(QCoapInternalRequest* request,
                                       uint currentBlockNumber,
                                       uint blockSize)
{
    qDebug() << "QCoapProtocol::onNextBlock()";
    request->setRequestToAskBlock(currentBlockNumber+1, blockSize);
    sendRequest(request);
}

/*void QCoapProtocolPrivate::sendAcknowledgment(QCoapReply* reply)
{
    if (!reply)
        return;

    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;
    QCoapInternalReply internalReply = internalReplies[reply].replies.last();

    copyRequest.setRequestForAck(internalReply.messageId(), internalReply.token());
    sendRequest(copyRequest);
}*/

void QCoapProtocolPrivate::sendAcknowledgment(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    QCoapInternalRequest ackRequest;
    QCoapInternalReply* internalReply = internalReplies[request].replies.last();

    ackRequest.initForAcknowledgment(internalReply->message().messageId(),
                                     internalReply->message().token());
    ackRequest.setConnection(request->connection());
    sendRequest(&ackRequest);
}

/*void QCoapProtocolPrivate::sendReset(QCoapReply* reply)
{
    if (!reply)
        return;

    qDebug() << "QCoapProtocol::sendReset()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;
    QCoapInternalReply internalReply = internalReplies[reply].replies.last();

    copyRequest.setRequestForReset(internalReply.messageId());
    sendRequest(copyRequest);
}*/

void QCoapProtocolPrivate::sendReset(QCoapInternalRequest* request)
{
    qDebug() << "QCoapProtocol::sendReset()";
    QCoapInternalRequest resetRequest;
    QCoapInternalReply* internalReply = internalReplies[request].replies.last();

    resetRequest.initForReset(internalReply->message().messageId());
    resetRequest.setConnection(request->connection());
    sendRequest(&resetRequest);
}

/*void QCoapProtocol::cancelObserve(QCoapReply* reply)
{
    if (!reply)
        return;

    Q_D(QCoapProtocol);
    QCoapInternalRequest* copyRequest = &(d->internalReplies[reply].request);
    copyRequest->setCancelObserve(true);
}*/

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

void QCoapProtocol::onAbortedRequest(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::onAbortedRequest()";
    Q_D(QCoapProtocol);
    QCoapInternalRequest* request = d->findInternalRequest(reply);
    if (request) {
        qDebug() << "REMOVE REQUEST";
        request->stopTransmission();
        d->internalReplies.remove(request);
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

/*bool QCoapProtocolPrivate::containsToken(const QByteArray& token)
{
    for (InternalMessagePair request : internalReplies.values()) {
        if (request.request.token() == token)
            return true;
    }

    return false;
}*/

bool QCoapProtocolPrivate::containsToken(const QByteArray& token)
{
    for (QCoapInternalRequest* request : internalReplies.keys()) {
        if (request->message().token() == token)
            return true;
    }

    return false;
}

/*bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (InternalMessagePair request : internalReplies.values()) {
        if (request.request.messageId() == id)
            return true;
    }

    return false;
}*/

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

void QCoapProtocolPrivate::setState(ProtocolState newState)
{
    state = newState;
}

QT_END_NAMESPACE

#include "moc_qcoapprotocol.cpp"
