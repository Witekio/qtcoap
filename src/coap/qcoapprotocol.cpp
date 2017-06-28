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

/*void QCoapProtocol::prepareToSendRequest(const QCoapInternalRequest& request, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyInternalRequest = request;

    //copyInternalRequest.setConnection(connection);

    if (copyInternalRequest.messageId() == 0) {
        do {
            copyInternalRequest.generateMessageId();
        } while (containsMessageId(copyInternalRequest.messageId()));
    }
    if (request.token().isEmpty()) {
        do {
            copyInternalRequest.generateToken();
        } while (containsToken(copyInternalRequest.token()));
    }

    qDebug() << "MID : " << copyInternalRequest.messageId()
             << " - TOKEN : " << copyInternalRequest.token();
    if (!findInternalRequestByToken(request.token()).isValid())
        d->internalReplies[copyInternalRequest];
    // QCoapInternalRequest::fromQCoapRequest(...);

    QByteArray requestFrame = encode(copyInternalRequest);
    connection->sendRequest(requestFrame);
}*/

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
            d->internalReplies[reply] = qMakePair(copyInternalRequest,
                                                  QList<QCoapInternalReply>());
    }

    d->sendRequest(copyInternalRequest);
}

void QCoapProtocolPrivate::sendRequest(const QCoapInternalRequest& request)
{
    Q_Q(QCoapProtocol);
    //QThread::msleep(1);
    QByteArray requestFrame = q->encode(request);
    request.connection()->sendRequest(requestFrame);
}

/*void QCoapProtocol::sendRequest()
{
    qDebug() << "QCoapProtocol::sendRequest()";
    Q_D(QCoapProtocol);
    QByteArray requestFrame = encode(d->internalRequest);
    d->connection->sendRequest(requestFrame);
}*/

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
    Q_Q(QCoapProtocol);
    q->handleFrame(frameQueue.head());
}

void QCoapProtocol::handleFrame(const QByteArray& frame)
{
    // TODO : check messageId/Token
    //qDebug() << "QCoapProtocol::handleFrame()";
    Q_D(QCoapProtocol);

    QCoapInternalReply internalReply = decode(frame);
    QCoapReply* reply = d->findReplyByToken(internalReply.token());

    if (!reply) {
        qDebug() << "No reply found (handleFrame)";
        return;
    }

    d->internalReplies[reply].second.push_back(internalReply);

    // Reply when the server ask an ACK
    if (d->internalReplies[reply].first.cancelObserve())
        sendReset(reply);
    else if (internalReply.type() == QCoapMessage::CONFIRMABLE)
        sendAcknowledgment(reply);

    // Ask next block or process the final reply
    if (internalReply.hasNextBlock())
        onNextBlock(reply, internalReply.currentBlockNumber());
    else
        onLastBlock(reply);

    // Take the next frame if needed
    d->frameQueue.dequeue();
    if (!d->frameQueue.isEmpty())
        d->handleFrame();
}

/*QCoapInternalRequest QCoapProtocol::findInternalRequestByToken(const QByteArray& token)
{
    Q_D(QCoapProtocol);
    for (QCoapInternalRequest request : d->internalReplies.keys()) {
        if (request.token() == token)
             return request;
    }

    return QCoapInternalRequest::invalidRequest();
}*/

QCoapReply* QCoapProtocolPrivate::findReplyByToken(const QByteArray& token)
{
    for (QCoapReply* reply : internalReplies.keys()) {
        if (internalReplies[reply].first.token() == token)
             return reply;
    }

    return nullptr;
}

/*void QCoapProtocol::onLastBlock(const QCoapInternalRequest& request)
{
    qDebug() << "QCoapProtocol::onLastBlock()";
    Q_D(QCoapProtocol);

    QList<QCoapInternalReply> replies = d->internalReplies[request];
    if (replies.isEmpty())
        return;

    QCoapInternalReply finalReply(replies.last());

    // If multiple blocks : append data from all blocks to the final reply
    if (replies.size() > 1) {
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
    d->internalReplies.remove(request);

    emit lastBlockReceived(finalReply);
}*/

void QCoapProtocol::onLastBlock(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::onLastBlock()";
    Q_D(QCoapProtocol);

    QList<QCoapInternalReply> replies = d->internalReplies[reply].second;
    QCoapInternalRequest request = d->internalReplies[reply].first;
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
        d->internalReplies.remove(reply);

    reply->updateFromInternalReply(finalReply);
    //emit lastBlockReceived(finalReply);
}

/*void QCoapProtocol::onNextBlock(const QCoapInternalRequest& request, uint currentBlockNumber)
{
    qDebug() << "QCoapProtocol::onNextBlock()";

    QCoapInternalRequest copyRequest = request;
    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    //prepareToSendRequest(request, copyRequest.connection());
}*/

void QCoapProtocol::onNextBlock(QCoapReply* reply, uint currentBlockNumber)
{
    //qDebug() << "QCoapProtocol::onNextBlock()";
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyRequest = d->internalReplies[reply].first;

    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    d->sendRequest(copyRequest);
}

void QCoapProtocol::sendAcknowledgment(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyRequest = d->internalReplies[reply].first;
    QCoapInternalReply internalReply = d->internalReplies[reply].second.last();

    copyRequest.setRequestForAck(internalReply.messageId(), internalReply.token());
    d->sendRequest(copyRequest);
}

void QCoapProtocol::sendReset(QCoapReply* reply)
{
    qDebug() << "QCoapProtocol::sendAcknowledgment()";
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyRequest = d->internalReplies[reply].first;
    QCoapInternalReply internalReply = d->internalReplies[reply].second.last();

    copyRequest.setRequestForReset(internalReply.messageId());
    d->sendRequest(copyRequest);
}

void QCoapProtocol::cancelObserve(QCoapReply* reply)
{
    Q_D(QCoapProtocol);
    QCoapInternalRequest* copyRequest = &(d->internalReplies[reply].first);
    copyRequest->setCancelObserve(true);
}

QByteArray QCoapProtocol::encode(const QCoapInternalRequest& request)
{
    return request.toQByteArray();
}

QCoapInternalReply QCoapProtocol::decode(const QByteArray& message)
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
    for (QPair<QCoapInternalRequest, QList<QCoapInternalReply>> request : internalReplies.values()) {
        if (request.first.token() == token)
            return true;
    }

    return false;
}

bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (QPair<QCoapInternalRequest, QList<QCoapInternalReply>> request : internalReplies.values()) {
        if (request.first.messageId() == id)
            return true;
    }

    return false;
}

void QCoapProtocolPrivate::setState(ProtocolState newState)
{
    state = newState;
}
