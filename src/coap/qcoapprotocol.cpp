#include "qcoapprotocol.h"
#include "qcoapprotocol_p.h"

QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
}

void QCoapProtocol::managePdu(const QByteArray& pdu)
{
    Q_D(QCoapProtocol);

    //d->internalReply.fromPdu(pdu);

    // TODO : check if this is the last block or not
    /*if (d->internalReply.hasNextBlock())
        onNextBlock();
    else
        onLastBlock();*/
}

/*void QCoapProtocol::initializeInternalRequest(QCoapRequest* request)
{
    Q_D(QCoapProtocol);
    d->internalRequest.initializeAttributesFrom(request);
}*/

void QCoapProtocol::onLastBlock()
{
    emit lastBlockReceived();
}

void QCoapProtocol::onNextBlock()
{
    //Q_D(QCoapProtocol);
    // TODO : make the request to make the next block
    //d->internalRequest.addOptionToAskBlock(d->internalReply.currentBlockNumber()+1);
    emit nextBlockAsked();
}
