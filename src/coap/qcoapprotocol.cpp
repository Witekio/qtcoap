#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QCoapProtocolPrivate::QCoapProtocolPrivate() :
    state(WAITING)
{
}

QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
}

void QCoapProtocol::sendRequest(QCoapReply* reply, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

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
}

void QCoapProtocolPrivate::sendRequest(const QCoapInternalRequest& request)
{
    QByteArray requestFrame = encode(request);
    request.connection()->sendRequest(requestFrame);
}

void QCoapProtocol::messageReceived(const QByteArray& frameReply)
{
    qDebug() << "QCoapProtocol::messageReceived()";
    Q_D(QCoapProtocol);

    d->frameQueue.enqueue(frameReply);
    if (d->frameQueue.size() == 1)
        d->handleFrame();
}

void QCoapProtocolPrivate::handleFrame()
{
    handleFrame(frameQueue.head());
}

void QCoapProtocolPrivate::handleFrame(const QByteArray& frame)
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
    else if (internalReply.type() == QCoapMessage::CONFIRMABLE)
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
}

QCoapReply* QCoapProtocolPrivate::findReplyByToken(const QByteArray& token)
{
    for (QCoapReply* reply : internalReplies.keys()) {
        if (internalReplies[reply].request.token() == token)
             return reply;
    }

    return nullptr;
}

QCoapReply* QCoapProtocolPrivate::findReplyByMessageId(quint16 messageId)
{
    for (QCoapReply* reply : internalReplies.keys()) {
        if (internalReplies[reply].request.messageId() == messageId)
             return reply;
    }

    return nullptr;
}

void QCoapProtocolPrivate::onLastBlock(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::onLastBlock()";

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

    reply->updateFromInternalReply(finalReply);
    //emit lastBlockReceived(finalReply);
}

void QCoapProtocolPrivate::onNextBlock(QCoapReply* reply, uint currentBlockNumber)
{
    //qDebug() << "QCoapProtocol::onNextBlock()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;

    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    sendRequest(copyRequest);
}

void QCoapProtocolPrivate::sendAcknowledgment(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;
    QCoapInternalReply internalReply = internalReplies[reply].replies.last();

    copyRequest.setRequestForAck(internalReply.messageId(), internalReply.token());
    sendRequest(copyRequest);
}

void QCoapProtocolPrivate::sendReset(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    QCoapInternalRequest copyRequest = internalReplies[reply].request;
    QCoapInternalReply internalReply = internalReplies[reply].replies.last();

    copyRequest.setRequestForReset(internalReply.messageId());
    sendRequest(copyRequest);
}

void QCoapProtocol::cancelObserve(QCoapReply* reply)
{
    Q_D(QCoapProtocol);
    QCoapInternalRequest* copyRequest = &(d->internalReplies[reply].request);
    copyRequest->setCancelObserve(true);
}

QByteArray QCoapProtocolPrivate::encode(const QCoapInternalRequest& request)
{
    return request.toQByteArray();
}

QCoapInternalReply QCoapProtocolPrivate::decode(const QByteArray& message)
{
    return  QCoapInternalReply::fromQByteArray(message);
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
    for (InternalMessagePair request : internalReplies.values()) {
        if (request.request.token() == token)
            return true;
    }

    return false;
}

bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (InternalMessagePair request : internalReplies.values()) {
        if (request.request.messageId() == id)
            return true;
    }

    return false;
}

void QCoapProtocolPrivate::setState(ProtocolState newState)
{
    state = newState;
}
