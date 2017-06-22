#include "qcoapreply.h"
#include "qcoapreply_p.h"
#include <QDebug>
#include <QtMath>

QCoapReplyPrivate::QCoapReplyPrivate() :
    status(INVALIDCODE),
    message(QCoapMessage())
{
}

QCoapReply::QCoapReply(QObject* parent) :
    QIODevice(* new QCoapReplyPrivate, parent)
{
    open(QIODevice::ReadOnly);
}

qint64 QCoapReply::readData(char* data, qint64 maxSize)
{
    QByteArray payload = d_func()->message.payload();

    qint64 len = qMin(maxSize, qint64(payload.size()) - pos());
    if (len <= 0)
        return qint64(0);
    memcpy(data, payload.constData() + pos(), len);

    return len;
}

qint64 QCoapReply::writeData(const char* data, qint64 maxSize)
{
    // The user cannot write to the reply
    return -1;
}

QCoapStatusCode QCoapReply::statusCode() const
{
    return d_func()->status;
}

QCoapMessage QCoapReply::message() const
{
    return d_func()->message;
}

void QCoapReply::updateWithInternalReply(const QCoapInternalReply& internalReply)
{
    d_func()->message.setPayload(internalReply.payload());
    d_func()->message.setType(internalReply.type());
    d_func()->message.setVersion(internalReply.version());
    d_func()->status = internalReply.statusCode();

    emit finished();
}

