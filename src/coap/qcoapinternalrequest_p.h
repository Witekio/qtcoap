#ifndef QCOAPINTERNALREQUEST_H
#define QCOAPINTERNALREQUEST_H

#include "qcoapglobal.h"
#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
//#include "qcoaprequest.h"

QT_BEGIN_NAMESPACE

class QCoapRequest;
class QCoapInternalRequestPrivate;
class QCoapInternalRequest : public QCoapInternalMessage
{
public:
    QCoapInternalRequest();
    QCoapInternalRequest(const QCoapRequest& request);

    static QCoapInternalRequest fromQCoapRequest(const QCoapRequest& request);
    QByteArray toQByteArray() const;

    void setOperation(QCoapOperation operation);

private:
    QCoapInternalRequestPrivate* d_func() const;
};

class QCoapInternalRequestPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalRequestPrivate();

    QCoapOperation operation;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
