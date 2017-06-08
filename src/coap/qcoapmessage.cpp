#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>

QCoapMessagePrivate::QCoapMessagePrivate() :
    version(1),
    type(QCoapMessage::NONCONFIRMABLE),
    messageId(0),
    token(0),
    tokenLength(0),
    payload(QByteArray())
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

/*QCoapMessage::QCoapMessage(const QCoapMessage& other)
{
    Q_D(QCoapMessage);
    d->type = other.type();
    d->messageId = other.messageId();
    d->tokenLength = other.tokenLength();
    d->token = other.token();
    d->options = other.options();
    d->payload = other.payload();
}*/

quint8 QCoapMessage::version() const
{
    return d_func()->version;
}

void QCoapMessage::setVersion(quint8 version)
{
    Q_D(QCoapMessage);
    d->version = version;
}

QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    return d_func()->type;
}

void QCoapMessage::setType(const QCoapMessageType& type)
{
    Q_D(QCoapMessage);
    d->type = type;
}

QByteArray QCoapMessage::token() const
{
    return d_func()->token;
}

void QCoapMessage::setToken(const QByteArray& token)
{
    Q_D(QCoapMessage);
    d->token = token;
    d->tokenLength = quint8(token.size());
}

quint8 QCoapMessage::tokenLength() const
{
    return d_func()->tokenLength;
}

quint16 QCoapMessage::messageId() const
{
    return d_func()->messageId;
}

void QCoapMessage::setMessageId(quint16 id)
{
    Q_D(QCoapMessage);
    d->messageId = id;
}

QByteArray QCoapMessage::payload() const
{
    return d_func()->payload;
}

void QCoapMessage::setPayload(const QByteArray& payload)
{
    Q_D(QCoapMessage);
    d->payload = payload;
}

void QCoapMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption* option = new QCoapOption(name, value);
    addOption(option);
}

void QCoapMessage::addOption(QCoapOption* option)
{
    Q_D(QCoapMessage);
    d->options.push_back(option);
}

QCoapOption* QCoapMessage::option(int index) const
{
    return d_func()->options.at(index);
}

int QCoapMessage::optionsLength() const
{
    return d_func()->options.length();
}
