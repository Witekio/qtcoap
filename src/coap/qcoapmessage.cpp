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
        quint32 blockNumber = 0;
        quint8 *optionData = (quint8 *)option->value().data();
        for (int i = 0; i < option->length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option->length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option->length()-1] & 0x8) == 0x8);
        d->blockSize = qPow(2, (optionData[option->length()-1] & 0x7) + 4);
        //qDebug() << option->length();
        /*qDebug() << "ADD BLOCK : " << d->currentBlockNumber
                 << " - " << d->hasNextBlock
                 << " - " << option->value().toHex();*/
    }

    d->options.push_back(option);
}

void QCoapMessage::removeOption(QCoapOption* option)
{
    Q_D(QCoapMessage);

    d->options.removeOne(option);
    delete option;
}

void QCoapMessage::removeOptionByName(QCoapOption::QCoapOptionName name)
{
    Q_D(QCoapMessage);

    for (QCoapOption* option : d->options) {
        if (option->name() == name) {
            removeOption(option);
            break;
        }
    }
}

void QCoapMessage::removeAllOptions()
{
    Q_D(QCoapMessage);

    qDeleteAll(d->options);
    d->options.clear();
}

bool QCoapMessage::hasNextBlock() const
{
    return d_func()->hasNextBlock;
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
    return d_func()->currentBlockNumber;
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
