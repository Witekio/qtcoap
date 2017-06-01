#include "qcoapreply.h"
#include "qcoapreply_p.h"
#include <QDebug>

QCoapReplyPrivate::QCoapReplyPrivate() :
    status_p(QCoapReply::INVALIDCODE)
{
}

QCoapReply::QCoapReply(QObject* parent) :
    QCoapMessage(* new QCoapReplyPrivate, parent)
{
}

void QCoapReply::fromPdu(const QByteArray& pdu)
{
    // TODO : finish to parse the pdu to reply
    Q_D(QCoapReply);
    quint8 *pduData = (quint8 *)pdu.data();

    // Parse Header and Token
    d->version_p = (pduData[0] >> 6) & 0x03;
    d->type_p = QCoapMessageType((pduData[0] >> 4) & 0x03);
    d->tokenLength_p = (pduData[0]) & 0x0F;
    d->status_p = static_cast<QCoapReplyStatusCode>(pduData[1]);
    d->messageId_p = (static_cast<quint16>(pduData[2]) << 8)
                       | static_cast<quint16>(pduData[3]);
    d->token_p = QByteArray(reinterpret_cast<char *>(pduData + 4),
                            d->tokenLength_p);

    // Parse Options
    // TODO : check for delta and length > 13 (extended fields)
    int i = 4 + d->tokenLength_p;
    quint8 lastOptionNumber = 0;
    while (quint8(pduData[i]) != 0xFF) {
        quint8 optionDelta = (pduData[i] >> 4) & 0x0F;
        quint8 optionNumber = lastOptionNumber + optionDelta;
        quint8 optionLength = pduData[i] & 0x0F;

        QByteArray optionValue("");
        if (optionLength != 0) {
            optionValue = QByteArray(reinterpret_cast<char *>(pduData + i + 1),
                                     optionLength);
        }

        addOption(QCoapOption::QCoapOptionName(optionNumber), optionValue);
        lastOptionNumber = optionNumber;
        i += (1 + optionLength);
    }

    // Parse Payload
    if (quint8(pduData[i]) == 0xFF)
        d->payload_p = pdu.right(pdu.length() - i - 1); // -1 because of 0xFF at the beginning
}

QByteArray QCoapReply::readData()
{
    return payload();
}

QCoapReply::QCoapReplyStatusCode QCoapReply::statusCode() const
{
    return d_func()->status_p;
}
