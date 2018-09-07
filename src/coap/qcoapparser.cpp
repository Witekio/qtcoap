/****************************************************************************
**
** Copyright (C) 2018 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtNetwork/qnetworkdatagram.h>
#include "qcoapparser_p.h"
#include "qcoapinternalmessage.h"

QT_BEGIN_NAMESPACE

/*!
    \internal

    \class QCoapParser
    \brief The QCoapParser class is responsible for encoding and decoding CoAP
    frames.

    \reentrant

    \sa QCoapProtocol
*/

/*!
    \internal
    Explicitly casts \a value to a char and appends it to the \a buffer.
*/
template<typename T>
void appendByte(QByteArray *buffer, T value) {
    buffer->append(static_cast<char>(value));
}

/*!
    \internal
    Encodes the \a message and \a method into a CoAP frame.

    For more details regarding the frame structure, refer to section
    \l{https://tools.ietf.org/html/rfc7252#section-3}{'Message format' of RFC 7252}.
*/
//! 0                   1                   2                   3
//! 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |Ver| T |  TKL  |      Code     |          Message ID           |
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Token (if any, TKL bytes) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |   Options (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//! |1 1 1 1 1 1 1 1|    Payload (if any) ...
//! +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
QByteArray QCoapParser::encode(const QCoapMessage &message, QtCoap::Method method) const
{
    QByteArray pdu;

    // Insert header
    appendByte(&pdu, (message.version()   << 6)           // CoAP version
                   | (message.type()      << 4)           // Message type
                   |  message.token().length());          // Token Length
    appendByte(&pdu,  method                    & 0xFF);  // Method code
    appendByte(&pdu, (message.messageId() >> 8) & 0xFF);  // Message ID
    appendByte(&pdu,  message.messageId()       & 0xFF);

    // Insert Token
    pdu.append(message.token());

    // Insert Options
    if (!message.options().isEmpty()) {
        // Sort options by ascending order
        // TODO: sort at insertion time in QCoapMessage, and assert that options are sorted here
        QVector<QCoapOption> options = message.options();
        std::sort(options.begin(), options.end(),
            [](const QCoapOption &a, const QCoapOption &b) -> bool {
                return a.name() < b.name();
        });

        quint8 lastOptionNumber = 0;
        for (const QCoapOption &option : qAsConst(options)) {

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            // Delta value > 12 : special values
            if (optionDelta > 268) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 269);
                optionDelta = 14;
                isOptionDeltaExtended = true;
            } else if (optionDelta > 12) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 13);
                optionDelta = 13;
                isOptionDeltaExtended = true;
            }

            quint16 optionLength = static_cast<quint16>(option.length());
            bool isOptionLengthExtended = false;
            quint8 optionLengthExtended = 0;

            // Length > 12 : special values
            if (optionLength > 268) {
                optionLengthExtended = static_cast<quint8>(optionLength - 269);
                optionLength = 14;
                isOptionLengthExtended = true;
            } else if (optionLength > 12) {
                optionLengthExtended = static_cast<quint8>(optionLength - 13);
                optionLength = 13;
                isOptionLengthExtended = true;
            }

            appendByte(&pdu, (optionDelta << 4) | (optionLength & 0x0F));

            if (isOptionDeltaExtended)
                appendByte(&pdu, optionDeltaExtended);
            if (isOptionLengthExtended)
                appendByte(&pdu, optionLengthExtended);

            pdu.append(option.value());

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!message.payload().isEmpty()) {
        appendByte(&pdu, 0xFF);
        pdu.append(message.payload());
    }

    return pdu;
}

/*!
    \internal
    Returns a QCoapMessage and QtCoap::ResponseCode after decoding the \a reply.
    For more details, see encode().
*/
std::pair<QCoapMessage, QtCoap::ResponseCode>
QCoapParser::decode(const QByteArray &reply) const
{
     QCoapMessage message;
     QtCoap::ResponseCode returnCode = QtCoap::InvalidCode;
     const quint8 *pduData = reinterpret_cast<const quint8 *>(reply.data());

     // Parse Header and Token
     message.setVersion((pduData[0] >> 6) & 0x03);
     message.setType(QCoapMessage::MessageType((pduData[0] >> 4) & 0x03));
     quint8 tokenLength = (pduData[0]) & 0x0F;
     returnCode = static_cast<QtCoap::ResponseCode>(pduData[1]);
     message.setMessageId(static_cast<quint16>((static_cast<quint16>(pduData[2]) << 8)
                                                  | static_cast<quint16>(pduData[3])));
     message.setToken(reply.mid(4, tokenLength));

     // Parse Options
     int i = 4 + tokenLength;
     quint16 lastOptionNumber = 0;
     while (i != reply.length() && pduData[i] != 0xFF) {
         quint16 optionDelta = ((pduData[i] >> 4) & 0x0F);
         quint16 optionLength = (pduData[i] & 0x0F);

         // Delta value > 12 : special values
         if (optionDelta == 13) {
             ++i;
             optionDelta = pduData[i] + 13;
         } else if (optionDelta == 14) {
             ++i;
             optionDelta = pduData[i] + 269;
         }

         // Delta length > 12 : special values
         if (optionLength == 13) {
             ++i;
             optionLength = pduData[i] + 13;
         } else if (optionLength == 14) {
             ++i;
             optionLength = pduData[i] + 269;
         }

         quint16 optionNumber = lastOptionNumber + optionDelta;
         message.addOption(QCoapOption::OptionName(optionNumber),
                           QByteArray(reply.data() + i + 1, optionLength));
         lastOptionNumber = optionNumber;
         i += 1 + optionLength;
     }

     // Parse Payload
     if (pduData[i] == 0xFF) {
         // Skip payload first byte marker '0xFF'
         QByteArray currentPayload = reply.mid(i + 1);
         message.setPayload(message.payload().append(currentPayload));
     }

     return {message, returnCode};
}

/*!
    \internal
    Sets block information of \a message from a descriptive block \a option.
    See \l{https://tools.ietf.org/html/rfc7959#section-2.3}{RFC 7959}.

    \note For block-wise transfer, the size of the block is expressed by a power
    of two. See
    \l{https://tools.ietf.org/html/rfc7959#section-2.2}{'Structure of a Block Option'}
    in RFC 7959 for more information.
*/
void QCoapParser::updateFromDescriptiveBlockOption(QCoapInternalMessage* message,
                                                   const QCoapOption& option) const
{
    if (!option.isValid())
        return;

    if (option.value().size() < 1) {
        qWarning("QtCoap: Invalid empty block option");
        return;
    }

    //! TODO Cover with tests
    const quint8 *optionData = reinterpret_cast<const quint8 *>(option.value().data());
    const quint8 lastByte = optionData[option.length() - 1];
    quint32 blockNumber = 0;

    for (int i = 0; i < option.length() - 1; ++i)
        blockNumber = (blockNumber << 8) | optionData[i];

    blockNumber = (blockNumber << 4) | (lastByte >> 4);
    message->setCurrentBlockNumber(blockNumber);
    message->setHasNextBlock((lastByte & 0x8) == 0x8);
    message->setBlockSize(static_cast<uint>(1u << ((lastByte & 0x7) + 4)));

    if (message->blockSize() > 1024)
        qWarning("QtCoap: Received a block size larger than 1024, something may be wrong.");
}

/*!
    \internal
    Builds and returns a Block option.

    The \a blockSize should range from 16 to 1024 and be a power of 2,
    computed as 2^(SZX + 4), with SZX ranging from 0 to 6. For more details,
    refer to the \l{https://tools.ietf.org/html/rfc7959#section-2.2}{RFC 7959}.
*/
QCoapOption QCoapParser::generateBlockOption(QCoapOption::OptionName name, uint blockNumber,
                                             uint blockSize, int fullPayloadSize) const
{
    //! TODO Cover this in tests
    Q_ASSERT((blockSize & (blockSize - 1)) == 0); // is a power of two
    Q_ASSERT(!(blockSize >> 11)); // blockSize <= 1024

    // NUM field: the relative number of the block within a sequence of blocks
    // 4, 12 or 20 bits (as little as possible)
    Q_ASSERT(!(blockNumber >> 20)); // Fits in 20 bits
    quint32 optionData = (blockNumber << 4);

    // SZX field: the size of the block
    // 3 bits, set to "log2(blockSize) - 4"
    optionData |= (blockSize >> 7)
                  ? ((blockSize >> 10) ? 6 : (3 + (blockSize >> 8)))
                  : (blockSize >> 5);

    // M field: whether more blocks are following
    // 1 bit
    if (name == QCoapOption::Block1
            && static_cast<int>((blockNumber + 1) * blockSize) < fullPayloadSize) {
        optionData |= 8;
    }

    QByteArray optionValue;
    Q_ASSERT(!(optionData >> 24));
    if (optionData > 0xFFFF)
        optionValue.append(static_cast<char>(optionData >> 16));
    if (optionData > 0xFF)
        optionValue.append(static_cast<char>((optionData >> 8) & 0xFF));
    optionValue.append(static_cast<char>(optionData & 0xFF));

    return QCoapOption(name, optionValue);
}

QT_END_NAMESPACE
