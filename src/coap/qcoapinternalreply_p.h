#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include "qcoapglobal.h"
#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"

QT_BEGIN_NAMESPACE

class QCoapInternalReplyPrivate;
class QCoapInternalReply : public QCoapInternalMessage
{
public:
    QCoapInternalReply();

    static QCoapInternalReply fromQByteArray(const QByteArray& reply);

    QCoapStatusCode statusCode() const;

private:
    QCoapInternalReplyPrivate* d_func() const;
};

class QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate();

    QCoapStatusCode statusCode;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
