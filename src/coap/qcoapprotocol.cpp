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

void QCoapProtocol::sendRequest(const QCoapInternalRequest& request, QCoapConnection* connection)
{
    Q_D(QCoapProtocol);

    d->connection = connection;
    d->internalRequest = request; //QCoapInternalRequest::fromQCoapRequest(request);

    connect(d->connection, SIGNAL(readyRead()), this, SLOT(messageReceived()));

    sendRequest();
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
    Q_D(QCoapProtocol);

    d->internalReply = decode(frame);

    if (d->internalReply.hasNextBlock())
        onNextBlock();
    else
        onLastBlock();
}

void QCoapProtocol::onLastBlock()
{
    emit lastBlockReceived(d_func()->internalReply);
}

void QCoapProtocol::onNextBlock()
{
    //Q_D(QCoapProtocol);
    // TODO : make the request to make the next block
    //d->internalRequest.addOptionToAskBlock(d->internalReply.currentBlockNumber()+1);
    emit nextBlockAsked();
}

QByteArray QCoapProtocol::encode(const QCoapInternalRequest& request)
{
    return request.toQByteArray();
}

QCoapInternalReply QCoapProtocol::decode(const QByteArray& message)
{
    return  QCoapInternalReply::fromQByteArray(message);
}
