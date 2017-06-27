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
        } while (containsMessageId(copyInternalRequest.messageId()));
    }
    if (copyInternalRequest.token().isEmpty()) {
        do {
            copyInternalRequest.generateToken();
        } while (containsToken(copyInternalRequest.token()));
    }

    qDebug() << "MID : " << copyInternalRequest.messageId()
             << " - TOKEN : " << copyInternalRequest.token();

    copyInternalRequest.setConnection(connection);
    // If this request does not already exist we add it to the map
    if (!findReplyByToken(copyInternalRequest.token())) {
            d->internalReplies[reply] = qMakePair(copyInternalRequest,
                                                  QList<QCoapInternalReply>());
    }

    d->sendRequest(copyInternalRequest);
}

void QCoapProtocolPrivate::sendRequest(const QCoapInternalRequest& request)
{
    Q_Q(QCoapProtocol);
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
    qDebug() << "void QCoapProtocol::messageReceived()";
    Q_D(QCoapProtocol);
    d->frameQueue.enqueue(frameReply);
    if (d->frameQueue.size() == 1)
        handleFrame();
}

void QCoapProtocol::handleFrame()
{
    Q_D(QCoapProtocol);
    handleFrame(d->frameQueue.dequeue());
}

void QCoapProtocol::handleFrame(const QByteArray& frame)
{
    // TODO : check messageId/Token
    qDebug() << "QCoapProtocol::handleFrame()";
    Q_D(QCoapProtocol);

    QCoapInternalReply internalReply = decode(frame);
    QCoapReply* reply = findReplyByToken(internalReply.token());

    //if (!request.isValid())
    if (!reply) {
        qDebug() << "No reply found (handleFrame)";
        return;
    }

    qDebug() << "- " << reply->request().token();
    d->internalReplies[reply].second.push_back(internalReply);

    if (internalReply.hasNextBlock())
        onNextBlock(reply, internalReply.currentBlockNumber());
    else
        onLastBlock(reply);
        //onLastBlock(request);

    if (!d->frameQueue.isEmpty())
        handleFrame();
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

QCoapReply* QCoapProtocol::findReplyByToken(const QByteArray& token)
{
    Q_D(QCoapProtocol);
    for (QCoapReply* reply : d->internalReplies.keys()) {
        if (d->internalReplies[reply].first.token() == token)
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
    d->internalReplies.remove(reply);

    reply->updateFromInternalReply(finalReply);
    //emit lastBlockReceived(finalReply);
}

/*void QCoapProtocol::onNextBlock(const QCoapInternalRequest& request, uint currentBlockNumber)
{
    qDebug() << "QCoapProtocol::onNextBlock(uint currentBlockNumber)";

    QCoapInternalRequest copyRequest = request;
    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    //prepareToSendRequest(request, copyRequest.connection());
}*/

void QCoapProtocol::onNextBlock(QCoapReply* reply, uint currentBlockNumber)
{
    qDebug() << "QCoapProtocol::onNextBlock(uint currentBlockNumber)";
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyRequest = d->internalReplies[reply].first;

    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    d->sendRequest(copyRequest);
}

/*void QCoapProtocol::startToSend()
{
    sendRequest();
}*/

QByteArray QCoapProtocol::encode(const QCoapInternalRequest& request)
{
    return request.toQByteArray();
}

QCoapInternalReply QCoapProtocol::decode(const QByteArray& message)
{
    return  QCoapInternalReply::fromQByteArray(message);
}

bool QCoapProtocol::containsToken(const QByteArray& token)
{
    Q_D(QCoapProtocol);
    for (QPair<QCoapInternalRequest, QList<QCoapInternalReply>> request : d->internalReplies.values()) {
        if (request.first.token() == token)
            return true;
    }

    return false;
}

bool QCoapProtocol::containsMessageId(quint16 id)
{
    Q_D(QCoapProtocol);
    for (QPair<QCoapInternalRequest, QList<QCoapInternalReply>> request : d->internalReplies.values()) {
        if (request.first.messageId() == id)
            return true;
    }

    return false;
}

void QCoapProtocolPrivate::setState(ProtocolState newState)
{
    state = newState;
}
