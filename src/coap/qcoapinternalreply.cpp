#include "qcoapinternalreply_p.h"

QCoapInternalReplyPrivate::QCoapInternalReplyPrivate():
    statusCode(InvalidCode)
{
}

QCoapInternalReplyPrivate::QCoapInternalReplyPrivate
    (const QCoapInternalReplyPrivate& other):
    QCoapInternalMessagePrivate(other),
    statusCode(other.statusCode)
{
}

QCoapInternalReply::QCoapInternalReply() :
    QCoapInternalMessage (*new QCoapInternalReplyPrivate)
{
}

QCoapInternalReply::QCoapInternalReply(const QCoapInternalReply& other) :
    QCoapInternalMessage(other)
{
}

QCoapInternalReply QCoapInternalReply::fromQByteArray(const QByteArray& reply)
{
    QCoapInternalReply internalReply;
    QCoapInternalReplyPrivate* d = internalReply.d_func();

    quint8 *pduData = reinterpret_cast<quint8 *>(const_cast<char *>(reply.data()));

    // Parse Header and Token
    d->version = (pduData[0] >> 6) & 0x03;
    d->type = QCoapMessageType((pduData[0] >> 4) & 0x03);
    quint8 tokenLength = (pduData[0]) & 0x0F;
    d->statusCode = static_cast<QCoapStatusCode>(pduData[1]);
    d->messageId = static_cast<quint16>((static_cast<quint16>(pduData[2]) << 8)
                                         | static_cast<quint16>(pduData[3]));
    d->token = QByteArray(reinterpret_cast<char *>(pduData + 4), tokenLength);

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

    // Parse Payload
    if (static_cast<quint8>(pduData[i]) == 0xFF) {
        QByteArray currentPayload = reply.right(reply.length() - i - 1); // -1 because of 0xFF at the beginning
        d->payload.append(currentPayload);
    }

    return internalReply;
}

void QCoapInternalReply::appendData(const QByteArray& data)
{
    d_func()->payload.append(data);
}

QCoapStatusCode QCoapInternalReply::statusCode() const
{
    return d_func()->statusCode;
}

QCoapInternalReplyPrivate* QCoapInternalReply::d_func() const
{
    return static_cast<QCoapInternalReplyPrivate*>(d_ptr);
}
