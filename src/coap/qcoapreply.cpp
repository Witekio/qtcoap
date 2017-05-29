#include "qcoapreply.h"
#include "qcoapreply_p.h"

QCoapReplyPrivate::QCoapReplyPrivate()
{
}

QCoapReply::QCoapReply()
{
}

void QCoapReply::fromPdu(const QByteArray& pdu)
{
    Q_UNUSED(pdu);
    // TODO
}

QByteArray QCoapReply::readData()
{
    return payload();
}

QCoapReply::QCoapReplyStatusCode QCoapReply::statusCode() const
{
    return d_func()->status_p;
}
