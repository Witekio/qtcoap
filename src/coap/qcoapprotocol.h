#ifndef QCOAPPROTOCOL_H
#define QCOAPPROTOCOL_H

#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"
#include "qcoapconnection.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QCoapProtocolPrivate;
class QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = 0);

    void handleFrame(const QByteArray& frame);
    void onLastBlock();
    void onNextBlock(uint currentBlockNumber);

    void sendRequest();
    void prepareToSendRequest(const QCoapInternalRequest& request, QCoapConnection* connection);
    QByteArray encode(const QCoapInternalRequest& request);
    QCoapInternalReply decode(const QByteArray& message);

signals:
    void lastBlockReceived(const QCoapInternalReply&);

public slots:
    void messageReceived();
    void startToSend();

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
