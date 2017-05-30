#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>

QCoapMessagePrivate::QCoapMessagePrivate() :
    type_p(QCoapMessage::NONCONFIRMABLE),
    messageId_p(0),
    token_p(0),
    payload_p(QByteArray())
{
}

QCoapMessage::QCoapMessage(QObject* parent) :
    QObject(* new QCoapMessagePrivate, parent)
{
}

QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd, QObject* parent) :
    QObject(dd, parent)
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
