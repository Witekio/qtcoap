#ifndef QCOAPINTERNALREQUEST_H
#define QCOAPINTERNALREQUEST_H

#include "qcoapglobal.h"
#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include "qcoapconnection.h"

QT_BEGIN_NAMESPACE

class QCoapRequest;
class QCoapInternalRequestPrivate;
class QCoapInternalRequest : public QCoapInternalMessage
{
public:
    QCoapInternalRequest();
    QCoapInternalRequest(const QCoapRequest& request);

    static QCoapInternalRequest invalidRequest();
    static QCoapInternalRequest fromQCoapRequest(const QCoapRequest& request);
    static QCoapInternalRequest createAcknowledgment(quint16 messageId, const QByteArray& token);
    static QCoapInternalRequest createReset(quint16 messageId);

    QByteArray toQByteArray() const;
    quint16 generateMessageId();
    QByteArray generateToken();
    void setRequestToAskBlock(uint blockNumber);

    bool isValid() const;
    bool cancelObserve() const;
    QCoapConnection* connection() const;
    void setOperation(QCoapOperation operation);
    void setConnection(QCoapConnection* connection);
    void setCancelObserve(bool cancelObserve);

    bool operator<(const QCoapInternalRequest& other) const;

private:
    QCoapInternalRequestPrivate* d_func() const;
};

class QCoapInternalRequestPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalRequestPrivate();

    QCoapOperation operation;
    QCoapConnection* connection;
    bool isValid;
    bool cancelObserve;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
