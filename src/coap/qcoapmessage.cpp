#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>

QCoapMessagePrivate::QCoapMessagePrivate() :
    QSharedData(),
    version(1),
    type(QCoapMessage::NONCONFIRMABLE),
    messageId(0),
    token(QByteArray()),
    payload(QByteArray())
{
}

QCoapMessagePrivate::QCoapMessagePrivate(const QCoapMessagePrivate& other) :
    QSharedData(other),
    version(other.version),
    type(other.type),
    messageId(other.messageId),
    token(other.token),
    options(other.options),
    payload(other.payload)
{
}

QCoapMessagePrivate::~QCoapMessagePrivate()
{
}

QCoapMessage::QCoapMessage() :
    d_ptr(new QCoapMessagePrivate)
{
}

QCoapMessage::QCoapMessage(const QCoapMessage& other) :
    d_ptr(other.d_ptr)
{
}

QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd) :
    d_ptr(&dd)
{
}

void QCoapMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption option(name, value);
    addOption(option);
}

void QCoapMessage::addOption(const QCoapOption& option)
{
    d_ptr->options.push_back(option);
}

void QCoapMessage::removeOption(const QCoapOption& option)
{
    d_ptr->options.removeOne(option);
}

void QCoapMessage::removeOptionByName(QCoapOption::QCoapOptionName name)
{
    for (QCoapOption option : d_ptr->options) {
        if (option.name() == name) {
            removeOption(option);
            break;
        }
    }
}

void QCoapMessage::removeAllOptions()
{
    d_ptr->options.clear();
}

quint8 QCoapMessage::version() const
{
    return d_ptr->version;
}

QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    return d_ptr->type;
}

QByteArray QCoapMessage::token() const
{
    return d_ptr->token;
}

quint8 QCoapMessage::tokenLength() const
{
    return static_cast<quint8>(d_ptr->token.length()); //tokenLength;
}

quint16 QCoapMessage::messageId() const
{
    return d_ptr->messageId;
}

QByteArray QCoapMessage::payload() const
{
    return d_ptr->payload;
}

QCoapOption QCoapMessage::option(int index) const
{
    return d_ptr->options.at(index);
}

int QCoapMessage::optionsLength() const
{
    return d_ptr->options.length();
}

void QCoapMessage::setVersion(quint8 version)
{
    if (d_ptr->version == version)
        return;

    d_ptr->version = version;
}


void QCoapMessage::setType(const QCoapMessageType& type)
{
    if (d_ptr->type == type)
        return;

    d_ptr->type = type;
}

void QCoapMessage::setToken(const QByteArray& token)
{
    if (d_ptr->token == token)
        return;

    d_ptr->token = token;
}

void QCoapMessage::setMessageId(quint16 id)
{
    if (d_ptr->messageId == id)
        return;

    d_ptr->messageId = id;
}

void QCoapMessage::setPayload(const QByteArray& payload)
{
    if (d_ptr->payload == payload)
        return;

    d_ptr->payload = payload;
}
