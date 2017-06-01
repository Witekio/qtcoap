#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>

QCoapMessagePrivate::QCoapMessagePrivate() :
    version_p(1),
    type_p(QCoapMessage::NONCONFIRMABLE),
    messageId_p(0),
    token_p(0),
    tokenLength_p(0),
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

/*QCoapMessage::QCoapMessage(const QCoapMessage& other)
{
    Q_D(QCoapMessage);
    d->type_p = other.type();
    d->messageId_p = other.messageId();
    d->tokenLength_p = other.tokenLength();
    d->token_p = other.token();
    d->options_p = other.options();
    d->payload_p = other.payload();
}*/

quint8 QCoapMessage::version() const
{
    return d_func()->version_p;
}

void QCoapMessage::setVersion(quint8 version)
{
    Q_D(QCoapMessage);
    d->version_p = version;
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

QByteArray QCoapMessage::token() const
{
    return d_func()->token_p;
}

void QCoapMessage::setToken(const QByteArray& token)
{
    Q_D(QCoapMessage);
    d->token_p = token;
    d->tokenLength_p = quint8(token.size());
}

quint8 QCoapMessage::tokenLength() const
{
    return d_func()->tokenLength_p;
}

quint16 QCoapMessage::messageId() const
{
    return d_func()->messageId_p;
}

void QCoapMessage::setMessageId(quint16 id)
{
    Q_D(QCoapMessage);
    d->messageId_p = id;
}

QByteArray QCoapMessage::payload() const
{
    return d_func()->payload_p;
}

void QCoapMessage::setPayload(const QByteArray& payload)
{
    Q_D(QCoapMessage);
    d->payload_p = payload;
}

void QCoapMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption* option = new QCoapOption(name, value);
    addOption(option);
}

void QCoapMessage::addOption(QCoapOption* option)
{
    Q_D(QCoapMessage);
    d->options_p.push_back(option);
}

QCoapOption* QCoapMessage::option(int index) const
{
    return d_func()->options_p.at(index);
}

int QCoapMessage::optionsLength() const
{
    return d_func()->options_p.length();
}
