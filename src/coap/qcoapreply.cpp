#include "qcoapreply.h"

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
    // TODO
    return QByteArray("");
}

QCoapReply::QCoapReplyStatusCode QCoapReply::statusCode() const
{
    return DELETED;
}

QByteArray QCoapReply::payload() const
{
    return QByteArray("");
}
