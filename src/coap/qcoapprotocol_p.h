#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include "qcoapprotocol.h"
#include "private/qobject_p.h"
#include <QList>
#include <QPair>
#include <QQueue>

QT_BEGIN_NAMESPACE

//struct InternalMessagePair {
//    QCoapInternalRequest request;
//    QList<QCoapInternalReply> replies;
//};

struct InternalMessagePair {
    QCoapReply* userReply;
    QList<QCoapInternalReply*> replies;
};

//typedef QMap<QCoapReply*, InternalMessagePair> InternalMessageMap;
typedef QMap<QCoapInternalRequest*, InternalMessagePair> InternalMessageMap;

class QCoapProtocolPrivate : public QObjectPrivate
{
public:
    enum ProtocolState {
        WAITING,
        SENDING
    };

    QCoapProtocolPrivate();

    void handleFrame();
    void handleFrame(const QByteArray& frame);
    //void onLastBlock(QCoapReply* request);
    void onLastBlock(QCoapInternalRequest* request);
    //void onNextBlock(QCoapReply* reply, uint currentBlockNumber);
    void onNextBlock(QCoapInternalRequest* request, uint currentBlockNumber, uint blockSize);
    //void sendAcknowledgment(QCoapReply* reply);
    void sendAcknowledgment(QCoapInternalRequest* request);
    //void sendReset(QCoapReply* reply);
    void sendReset(QCoapInternalRequest* request);
    QByteArray encode(const QCoapInternalRequest& request);
    QCoapInternalReply* decode(const QByteArray& message);

    void setState(ProtocolState newState);
    void sendRequest(QCoapInternalRequest* request);
    bool containsMessageId(quint16 id);
    bool containsToken(const QByteArray& token);
    //QCoapReply* findReplyByToken(const QByteArray& token);
    QCoapInternalRequest* findRequestByToken(const QByteArray& token);
    //QCoapReply* findReplyByMessageId(quint16 messageId);
    QCoapInternalRequest* findRequestByMessageId(quint16 messageId);

    InternalMessageMap internalReplies;
    QQueue<QByteArray> frameQueue;
    ProtocolState state;
    quint16 blockSize;

    uint ackTimeout;
    double ackRandomFactor;
    uint maxRetransmit;
    //uint maxTransmitSpan;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
