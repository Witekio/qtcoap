#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>

QCoapMessagePrivate::QCoapMessagePrivate() :
    version(1),
    type(QCoapMessage::NONCONFIRMABLE),
    messageId(0),
    token(0),
    tokenLength(0),
    payload(QByteArray()),
    currentBlockNumber(0),
    hasNextBlock(false)
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

void QCoapMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption* option = new QCoapOption(name, value);
    addOption(option);
}

void QCoapMessage::addOption(QCoapOption* option)
{
    Q_D(QCoapMessage);

    // If it is a BLOCK option, we need to know the actuel block number
    if (option->name() == QCoapOption::BLOCK1
        || option->name() == QCoapOption::BLOCK2) {
        quint8 *optionData = (quint8 *)option->value().data();
        quint16 blockNumber = ((static_cast<quint16>(optionData[0]) << 8)
                | (static_cast<quint16>(optionData[1]))) >> 4;
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[1] & 0x8) == 0x8);
    }

    d->options.push_back(option);
}

// TODO : autotest for blockwise transfer
bool QCoapMessage::hasNextBlock() const
{
    Q_D(const QCoapMessage);

    return d->hasNextBlock;
}

quint8 QCoapMessage::version() const
{
    return d_func()->version;
}

QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    return d_func()->type;
}

QByteArray QCoapMessage::token() const
{
    return d_func()->token;
}

quint8 QCoapMessage::tokenLength() const
{
    return d_func()->tokenLength;
}

quint16 QCoapMessage::messageId() const
{
    return d_func()->messageId;
}

QByteArray QCoapMessage::payload() const
{
    return d_func()->payload;
}

QCoapOption* QCoapMessage::option(int index) const
{
    return d_func()->options.at(index);
}

int QCoapMessage::optionsLength() const
{
    return d_func()->options.length();
}

uint QCoapMessage::currentBlockNumber() const
{
    return currentBlockNumber();
}

void QCoapMessage::setVersion(quint8 version)
{
    Q_D(QCoapMessage);
    if (d->version == version)
        return;

    d->version = version;
}


void QCoapMessage::setType(const QCoapMessageType& type)
{
    Q_D(QCoapMessage);
    if (d->type == type)
        return;

    d->type = type;
}

void QCoapMessage::setToken(const QByteArray& token)
{
    Q_D(QCoapMessage);
    if (d->token == token)
        return;

    d->token = token;
    d->tokenLength = quint8(token.size());
}

void QCoapMessage::setMessageId(quint16 id)
{
    Q_D(QCoapMessage);
    if (d->messageId == id)
        return;

    d->messageId = id;
}

void QCoapMessage::setPayload(const QByteArray& payload)
{
    Q_D(QCoapMessage);
    if (d->payload == payload)
        return;

    d->payload = payload;
}
