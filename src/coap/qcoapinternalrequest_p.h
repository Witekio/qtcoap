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
    QByteArray toQByteArray() const;

    quint16 generateMessageId();
    QByteArray generateToken();
    void setRequestToAskBlock(uint blockNumber);

    bool isValid() const;
    QCoapConnection* connection() const;
    void setOperation(QCoapOperation operation);
    void setConnection(QCoapConnection* connection);

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
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
