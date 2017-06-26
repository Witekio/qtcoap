#ifndef QCOAPPROTOCOL_H
#define QCOAPPROTOCOL_H

#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoapconnection.h"
#include "qcoapreply.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QCoapProtocolPrivate;
class QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = 0);

    void handleFrame(const QByteArray& frame);
    //void onLastBlock(const QCoapInternalRequest& request);
    void onLastBlock(QCoapReply* request);
    //void onNextBlock(const QCoapInternalRequest& request, uint currentBlockNumber);
    void onNextBlock(QCoapReply* reply, uint currentBlockNumber);

    //void sendRequest();
    //void prepareToSendRequest(const QCoapInternalRequest& request, QCoapConnection* connection);
    void sendRequest(QCoapReply* reply, QCoapConnection* connection);
    QByteArray encode(const QCoapInternalRequest& request);
    QCoapInternalReply decode(const QByteArray& message);

signals:
    void lastBlockReceived(const QCoapInternalReply&);
    void frameQueueFilled();

public slots:
    void messageReceived(const QByteArray& frameReply);
    //void startToSend();

private:
    void handleFrame();
    bool containsMessageId(quint16 id);
    bool containsToken(const QByteArray& token);
    //QCoapInternalRequest findInternalRequestByToken(const QByteArray& token);
    QCoapReply* findReplyByToken(const QByteArray& token);

    Q_DECLARE_PRIVATE(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
