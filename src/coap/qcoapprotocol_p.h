#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include "qcoapprotocol.h"
#include "private/qobject_p.h"
#include <QList>
#include <QPair>
#include <QQueue>

QT_BEGIN_NAMESPACE

struct InternalMessagePair {
    QCoapReply* userReply;
    QList<QCoapInternalReply*> replies;
};

typedef QMap<QCoapInternalRequest*, InternalMessagePair> InternalMessageMap;

class QCoapProtocolPrivate : public QObjectPrivate
{
public:
    QCoapProtocolPrivate();

    void handleFrame();
    void handleFrame(const QByteArray& frame);
    void onLastBlock(QCoapInternalRequest* request);
    void onNextBlock(QCoapInternalRequest* request, uint currentBlockNumber, uint blockSize);
    void sendAcknowledgment(QCoapInternalRequest* request);
    void sendReset(QCoapInternalRequest* request);
    QByteArray encode(QCoapInternalRequest* request);
    QCoapInternalReply* decode(const QByteArray& message);

    void sendRequest(QCoapInternalRequest* request);
    void resendRequest(QCoapInternalRequest* request);
    bool containsMessageId(quint16 id);
    bool containsToken(const QByteArray& token);
    QCoapInternalRequest* findInternalRequestByToken(const QByteArray& token);
    QCoapInternalRequest* findInternalRequestByMessageId(quint16 messageId);
    QCoapInternalRequest* findInternalRequestByReply(QCoapReply* reply);

    void messageReceived(const QByteArray& frameReply);
    void onAbortedRequest(QCoapReply* reply);

    InternalMessageMap internalReplies;
    QQueue<QByteArray> frameQueue;
    quint16 blockSize;

    uint ackTimeout;
    double ackRandomFactor;
    uint maxRetransmit;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
