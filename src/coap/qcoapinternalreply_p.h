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

private:
    QCoapInternalReplyPrivate* d_ptr;
};

class QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate();

    QCoapReply::QCoapReplyStatusCode statusCode;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
