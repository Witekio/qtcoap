#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QCoapProtocolPrivate::QCoapProtocolPrivate() :
    connection(nullptr)
{
}

QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
}

void QCoapProtocol::prepareToSendRequest(const QCoapInternalRequest& request, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    d->connection = connection;
    d->internalRequest = request; //QCoapInternalRequest::fromQCoapRequest(request);

    // TODO : check if message ID and token are not in use
    if (d->internalRequest.messageId() == 0)
        d->internalRequest.generateMessageId();
    if (d->internalRequest.token().isEmpty())
        d->internalRequest.generateToken();

    connect(d->connection, SIGNAL(readyRead()), this, SLOT(messageReceived()));
}

void QCoapProtocol::sendRequest()
{
    Q_D(QCoapProtocol);
    QByteArray requestFrame = encode(d->internalRequest);
    d->connection->sendRequest(requestFrame);
}

void QCoapProtocol::messageReceived()
{
    // TODO : check messageId/Token
    handleFrame(d_func()->connection->readReply());
}

void QCoapProtocol::handleFrame(const QByteArray& frame)
{
    //qDebug() << "QCoapProtocol::handleFrame(const QByteArray& frame)";
    Q_D(QCoapProtocol);

    QCoapInternalReply reply = decode(frame);
    d->internalReply.push_back(reply);

    if (reply.hasNextBlock())
        onNextBlock(reply.currentBlockNumber());
    else
        onLastBlock();
}

void QCoapProtocol::onLastBlock()
{
    //qDebug() << "QCoapProtocol::onLastBlock()";
    Q_D(QCoapProtocol);

    if (d->internalReply.isEmpty())
        return;

    QCoapInternalReply finalReply(d->internalReply.last());

    // If multiple blocks : append data from all blocks to the final reply
    if (d->internalReply.size() > 1) {
        qSort(std::begin(d->internalReply), std::end(d->internalReply),
              [](const QCoapInternalReply& a, const QCoapInternalReply& b) -> bool {
            return (a.currentBlockNumber() < b.currentBlockNumber());
        });

        QByteArray finalPayload;
        for (const QCoapInternalReply& reply : d->internalReply) {
            QByteArray replyPayload = reply.payload();
            if (replyPayload.isEmpty())
                continue;

            finalPayload.append(replyPayload);
        }

        finalReply.setPayload(finalPayload);
    }

    emit lastBlockReceived(finalReply);
}

void QCoapProtocol::onNextBlock(uint currentBlockNumber)
{
    //qDebug() << "QCoapProtocol::onNextBlock(uint currentBlockNumber)";

    Q_D(QCoapProtocol);
    d->internalRequest.setRequestToAskBlock(currentBlockNumber+1);
    sendRequest();
}

void QCoapProtocol::startToSend()
{
    sendRequest();
}

QByteArray QCoapProtocol::encode(const QCoapInternalRequest& request)
{
    return request.toQByteArray();
}

QCoapInternalReply QCoapProtocol::decode(const QByteArray& message)
{
    return  QCoapInternalReply::fromQByteArray(message);
}
