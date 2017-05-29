#include "qcoapmessage.h"
#include "qcoapmessage_p.h"

QCoapMessagePrivate::QCoapMessagePrivate()
{
}

QCoapMessage::QCoapMessage(QObject* parent) :
    QObject(parent)
{
}

QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    return d_func()->type_p;
}

void QCoapMessage::setType(const QCoapMessageType& type)
{
    Q_D(QCoapMessage);
    d->type_p = type;
}

qint64 QCoapMessage::token() const
{
    return d_func()->token_p;
}

qint16 QCoapMessage::messageId() const
{
    return d_func()->messageId_p;
}

QByteArray QCoapMessage::payload() const
{
    return d_func()->payload_p;
}
