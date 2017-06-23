#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QCoapProtocolPrivate::QCoapProtocolPrivate()
{
}

QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
}

// TODO : rename to sendRequest
void QCoapProtocol::prepareToSendRequest(const QCoapInternalRequest& request, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    QCoapInternalRequest copyInternalRequest = request;

    //copyInternalRequest.setConnection(connection);

    // TODO : check if message ID and token are not in use
    if (copyInternalRequest.messageId() == 0)
        copyInternalRequest.generateMessageId();
    if (request.token().isEmpty())
        copyInternalRequest.generateToken();

    if (!findInternalRequestByToken(request.token()).isValid())
        d->internalReplies[copyInternalRequest];
    // QCoapInternalRequest::fromQCoapRequest(...);

    QByteArray requestFrame = encode(copyInternalRequest);
    connection->sendRequest(requestFrame);
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
    handleFrame(frameReply);
}

void QCoapProtocol::handleFrame(const QByteArray& frame)
{
    // TODO : check messageId/Token
    qDebug() << "QCoapProtocol::handleFrame()";
    Q_D(QCoapProtocol);

    QCoapInternalReply reply = decode(frame);
    QCoapInternalRequest request = findInternalRequestByToken(reply.token());
    if (!request.isValid())
        return;

    d->internalReplies[request].push_back(reply);

    if (reply.hasNextBlock())
        onNextBlock(request, reply.currentBlockNumber());
    else
        onLastBlock(request);
}

QCoapInternalRequest QCoapProtocol::findInternalRequestByToken(const QByteArray& token)
{
    Q_D(QCoapProtocol);
    for (QCoapInternalRequest request : d->internalReplies.keys()) {
        if (request.token() == token)
             return request;
    }

    return QCoapInternalRequest::invalidRequest();
}

void QCoapProtocol::onLastBlock(const QCoapInternalRequest& request)
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
}

void QCoapProtocol::onNextBlock(const QCoapInternalRequest& request, uint currentBlockNumber)
{
    qDebug() << "QCoapProtocol::onNextBlock(uint currentBlockNumber)";

    QCoapInternalRequest copyRequest = request;
    copyRequest.setRequestToAskBlock(currentBlockNumber+1);
    //prepareToSendRequest(request, copyRequest.connection());
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
