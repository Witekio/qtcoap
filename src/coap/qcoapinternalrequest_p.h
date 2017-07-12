#ifndef QCOAPINTERNALREQUEST_H
#define QCOAPINTERNALREQUEST_H

#include "qcoapglobal.h"
#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include "qcoapconnection.h"

#include <QtCore/qglobal.h>
#include <QTimer>
#include <QUrl>

QT_BEGIN_NAMESPACE

class QCoapRequest;
class QCoapInternalRequestPrivate;
class QCoapInternalRequest : public QCoapInternalMessage
{
    Q_OBJECT
public:
    QCoapInternalRequest(QObject* parent = Q_NULLPTR);
    QCoapInternalRequest(const QCoapRequest& request, QObject* parent = Q_NULLPTR);

    static QCoapInternalRequest invalidRequest();
    void initForAcknowledgment(quint16 messageId, const QByteArray& token);
    void initForReset(quint16 messageId);

    QByteArray toQByteArray() const;
    quint16 generateMessageId();
    QByteArray generateToken();
    void setRequestToAskBlock(uint blockNumber, uint blockSize);
    void setRequestToSendBlock(uint blockNumber, uint blockSize);

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption& option);
    void addUriOptions(const QUrl& uri, const QUrl& proxyUri = QUrl());

    QUrl targetUri() const;
    bool isValid() const;
    QCoapOperation operation() const;
    bool cancelObserve() const;
    QCoapConnection* connection() const;
    QTimer* timer() const;
    uint retransmissionCounter() const;
    void setOperation(QCoapOperation operation);
    void setConnection(QCoapConnection* connection);
    void setCancelObserve(bool cancelObserve);

    void setTargetUri(QUrl targetUri);
    void setTimeout(int timeout);
    void beginTransmission();
    void stopTransmission();

    bool operator<(const QCoapInternalRequest& other) const;

signals:
    void timeout(QCoapInternalRequest*);

private:
    Q_DECLARE_PRIVATE(QCoapInternalRequest)
    Q_PRIVATE_SLOT(d_func(), void _q_timeout())
};

class QCoapInternalRequestPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalRequestPrivate();

    QUrl targetUri;
    QCoapOperation operation;
    QCoapConnection* connection;
    bool isValid;
    bool cancelObserve;
    QByteArray fullPayload;

    uint retransmissionCounter;
    int timeout;
    QTimer* timer;

    void _q_timeout();

    Q_DECLARE_PUBLIC(QCoapInternalRequest)
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
