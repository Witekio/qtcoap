#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include "qcoapreply.h"

QT_BEGIN_NAMESPACE

class QCoapInternalReplyPrivate;
class QCoapInternalReply : public QCoapInternalMessage
{
public:
    QCoapInternalReply();

    static QCoapInternalReply fromQByteArray(const QByteArray& reply);

    QCoapReply::QCoapReplyStatusCode statusCode() const;

private:
    QCoapInternalReplyPrivate* d_func() const;
};

class QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate();

    QCoapReply::QCoapReplyStatusCode statusCode;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
