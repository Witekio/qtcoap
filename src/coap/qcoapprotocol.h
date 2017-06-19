#ifndef QCOAPPROTOCOL_H
#define QCOAPPROTOCOL_H

#include <QObject>
#include "qcoaprequest.h"

QT_BEGIN_NAMESPACE

class QCoapProtocolPrivate;
class QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = 0);

    void managePdu(const QByteArray& pdu);
    void initializeInternalRequest(QCoapRequest* request);
    void onLastBlock();
    void onNextBlock();

signals:
    void lastBlockReceived();
    void nextBlockAsked();

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
