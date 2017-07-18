#include "qcoapinternalreply_p.h"
#include <QtMath>
#include <QDebug>

QT_BEGIN_NAMESPACE

QCoapInternalReplyPrivate::QCoapInternalReplyPrivate():
    statusCode(InvalidCoapCode)
{
}

QCoapInternalReplyPrivate::QCoapInternalReplyPrivate
    (const QCoapInternalReplyPrivate& other):
    QCoapInternalMessagePrivate(other),
    statusCode(other.statusCode)
{
}

/*!
    \internal

    \class QCoapInternalReply
    \brief The QCoapInternalReply class contains data related to
    a received message.

    \reentrant

    \sa QCoapInternalMessage, QCoapInternalRequest
*/

/*!
    \internal
    Constructs a new QCoapInternalReply with \a parent as the parent obect.
*/
QCoapInternalReply::QCoapInternalReply(QObject* parent) :
    QCoapInternalMessage (*new QCoapInternalReplyPrivate, parent)
{
}

/*!
    \internal
    Constructs a copy of \a other with \a parent as the parent obect.
*/
QCoapInternalReply::QCoapInternalReply(const QCoapInternalReply& other, QObject* parent) :
    QCoapInternalMessage(other, parent)
{
    Q_D(QCoapInternalReply);
    d->statusCode = other.statusCode();
}

/*!
    \internal
    Create a QCoapInternalReply from the \a reply byte array which need
    to be a coap reply frame.
*/
QCoapInternalReply QCoapInternalReply::fromQByteArray(const QByteArray& reply)
{
    QCoapInternalReply internalReply;
    QCoapInternalReplyPrivate* d = internalReply.d_func();

    quint8 *pduData = reinterpret_cast<quint8 *>(const_cast<char *>(reply.data()));

    // Parse Header and Token
    d->message.setVersion((pduData[0] >> 6) & 0x03);
    d->message.setType(QCoapMessage::QCoapMessageType((pduData[0] >> 4) & 0x03));
    quint8 tokenLength = (pduData[0]) & 0x0F;
    d->statusCode = static_cast<QCoapStatusCode>(pduData[1]);
    d->message.setMessageId(static_cast<quint16>((static_cast<quint16>(pduData[2]) << 8)
                                         | static_cast<quint16>(pduData[3])));
    d->message.setToken(QByteArray(reinterpret_cast<char *>(pduData + 4), tokenLength));

    // Parse Options
    int i = 4 + tokenLength;
    quint16 lastOptionNumber = 0;
    while (i != reply.length() && static_cast<quint8>(pduData[i]) != 0xFF) {
        quint16 optionDelta = static_cast<quint16>((pduData[i] >> 4) & 0x0F);
        quint8 optionDeltaExtended = 0;
        quint16 optionLength = static_cast<quint16>(pduData[i] & 0x0F);
        quint8 optionLengthExtended = 0;

        //qDebug() << QString::number(static_cast<quint8>(pduData[i]), 16);
        // Delta value > 12 : special values
        if (optionDelta == 13) {
            ++i;
            optionDeltaExtended = pduData[i];
            optionDelta = optionDeltaExtended + 13;
        } else if (optionDelta == 14) {
            ++i;
            optionDeltaExtended = pduData[i];
            optionDelta = optionDeltaExtended + 269;
        }

        // Delta length > 12 : special values
        if (optionLength == 13) {
            ++i;
            optionLengthExtended = pduData[i];
            optionLength = optionLengthExtended + 13;
        } else if (optionLength == 14) {
            ++i;
            optionLengthExtended = pduData[i];
            optionLength = optionLengthExtended + 269;
        }

        QByteArray optionValue("");
        if (optionLength != 0) {
            optionValue = QByteArray(reinterpret_cast<char *>(pduData + i + 1),
                                     optionLength);
        }

        quint16 optionNumber = lastOptionNumber + optionDelta;
        internalReply.addOption(QCoapOption::QCoapOptionName(optionNumber), optionValue);
        lastOptionNumber = optionNumber;
        i += (1 + optionLength);
    }

    qDebug() << "options length : " << internalReply.message().optionsLength();

    // Parse Payload
    if (static_cast<quint8>(pduData[i]) == 0xFF) {
        QByteArray currentPayload = reply.right(reply.length() - i - 1); // -1 because of 0xFF at the beginning
        d->message.setPayload(d->message.payload().append(currentPayload));
    }

    return internalReply;
}

/*!
    \internal
    Append the given \a data byte array to the current payload.
*/
void QCoapInternalReply::appendData(const QByteArray& data)
{
    d_func()->message.setPayload(d_func()->message.payload().append(data));
}

/*!
    \internal
    Adds the given coap \a option and sets block parameters if needed.
*/
void QCoapInternalReply::addOption(const QCoapOption& option)
{
    Q_D(QCoapInternalReply);
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::Block2CoapOption) {
        quint32 blockNumber = 0;
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d->blockSize = static_cast<uint>(qPow(2, (optionData[option.length()-1] & 0x7) + 4));
    }

    d->message.addOption(option);
}

/*!
    \internal
    Returns true if it is not the last block.
*/
int QCoapInternalReply::wantNextBlock()
{
    QCoapOption option = d_func()->message.findOptionByName(QCoapOption::Block1CoapOption);
    if (option.name() != QCoapOption::InvalidCoapOption) {
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());

        bool hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        //int blockSize = qPow(2, (optionData[option.length()-1] & 0x7) + 4);

        if (hasNextBlock) {
            quint32 blockNumber = 0;
            for (int i = 0; i < option.length() - 1; ++i)
                blockNumber = (blockNumber << 8) | optionData[i];
            blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
            return static_cast<int>(blockNumber) + 1;
        } else
            return -1;
    }

    return -1;
}

/*!
    \internal
    Returns the status code of the reply.
*/
QCoapStatusCode QCoapInternalReply::statusCode() const
{
    return d_func()->statusCode;
}

QT_END_NAMESPACE
