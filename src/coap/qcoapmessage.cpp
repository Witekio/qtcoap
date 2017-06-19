#include "qcoapmessage.h"
#include "qcoapmessage_p.h"
#include <QDebug>
#include <QtCore/QtMath>

QCoapMessagePrivate::QCoapMessagePrivate() :
    version(1),
    type(QCoapMessage::NONCONFIRMABLE),
    messageId(0),
    token(0),
    tokenLength(0),
    payload(QByteArray()),
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapMessage::QCoapMessage() :
    d_ptr(new QCoapMessagePrivate)
{
}

QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd) :
    d_ptr(&dd)
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
    QCoapOption option(name, value);
    addOption(option);
}

void QCoapMessage::addOption(const QCoapOption& option)
{
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::BLOCK1
        || option.name() == QCoapOption::BLOCK2) {
        quint32 blockNumber = 0;
        quint8 *optionData = (quint8 *)option.value().data();
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d_ptr->currentBlockNumber = blockNumber;
        d_ptr->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d_ptr->blockSize = qPow(2, (optionData[option.length()-1] & 0x7) + 4);
        //qDebug() << option->length();
        /*qDebug() << "ADD BLOCK : " << d->currentBlockNumber
                 << " - " << d->hasNextBlock
                 << " - " << option->value().toHex();*/
    }

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
    qDeleteAll(d_ptr->options);
    d_ptr->options.clear();
}

bool QCoapMessage::hasNextBlock() const
{
    return d_ptr->hasNextBlock;
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
    return d_ptr->tokenLength;
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

uint QCoapMessage::currentBlockNumber() const
{
    return d_ptr->currentBlockNumber;
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
    d_ptr->tokenLength = quint8(token.size());
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
