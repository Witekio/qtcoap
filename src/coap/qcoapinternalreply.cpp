/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcoapinternalreply_p.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

QCoapInternalReplyPrivate::QCoapInternalReplyPrivate():
    statusCode(QtCoap::Invalid)
{
}

/*!
    \internal

    \class QCoapInternalReply
    \brief The QCoapInternalReply class contains data related to
    a received message.

    It is a subclass of QCoapInternalMessage.

    \reentrant

    \sa QCoapInternalMessage, QCoapInternalRequest
*/

/*!
    \internal
    Constructs a new QCoapInternalReply with \a parent as the parent object.
*/
QCoapInternalReply::QCoapInternalReply(QObject *parent) :
    QCoapInternalMessage (*new QCoapInternalReplyPrivate, parent)
{
}

/*!
    \internal
    Constructs a copy of \a other with \a parent as the parent obect.
*/
QCoapInternalReply::QCoapInternalReply(const QCoapInternalReply &other, QObject *parent) :
    QCoapInternalMessage(other, parent)
{
    Q_D(QCoapInternalReply);
    d->statusCode = other.statusCode();
}

/*!
    \internal
    Creates a QCoapInternalReply from the CoAP \a reply frame, as a QByteArray.

    For more details, refer to section
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
QCoapInternalReply QCoapInternalReply::fromQByteArray(const QByteArray &reply)
{
    QCoapInternalReply internalReply;
    QCoapInternalReplyPrivate *d = internalReply.d_func();

    const quint8 *pduData = reinterpret_cast<const quint8 *>(reply.data());

    // Parse Header and Token
    d->message.setVersion((pduData[0] >> 6) & 0x03);
    d->message.setType(QCoapMessage::MessageType((pduData[0] >> 4) & 0x03));
    quint8 tokenLength = (pduData[0]) & 0x0F;
    d->statusCode = static_cast<QtCoap::StatusCode>(pduData[1]);
    d->message.setMessageId(static_cast<quint16>((static_cast<quint16>(pduData[2]) << 8)
                                                  | static_cast<quint16>(pduData[3])));
    d->message.setToken(QByteArray::fromRawData(reply.data() + 4, tokenLength));

    // Parse Options
    int i = 4 + tokenLength;
    quint16 lastOptionNumber = 0;
    while (i != reply.length() && pduData[i] != 0xFF) {
        quint16 optionDelta = ((pduData[i] >> 4) & 0x0F);
        quint8 optionDeltaExtended = 0;
        quint16 optionLength = (pduData[i] & 0x0F);
        quint8 optionLengthExtended = 0;

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

        quint16 optionNumber = lastOptionNumber + optionDelta;
        internalReply.addOption(QCoapOption::OptionName(optionNumber),
                                QByteArray::fromRawData(reply.data() + i + 1,
                                                        optionLength));
        lastOptionNumber = optionNumber;
        i += 1 + optionLength;
    }

    // Parse Payload
    if (pduData[i] == 0xFF) {
        // -1 because of 0xFF at the beginning
        QByteArray currentPayload = reply.right(reply.length() - i - 1);
        d->message.setPayload(d->message.payload().append(currentPayload));
    }

    return internalReply;
}

/*!
    \internal
    Appends the given \a data byte array to the current payload.
*/
void QCoapInternalReply::appendData(const QByteArray &data)
{
    Q_D(QCoapInternalReply);
    d->message.setPayload(d->message.payload().append(data));
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.

    For block-wise transfer, the size of the block is expressed in power
    of two. See
    \l{https://tools.ietf.org/html/rfc7959#section-2.2}{'Structure of a Block Option'}
    of the RFC 7959 for more information.
*/
void QCoapInternalReply::addOption(const QCoapOption &option)
{
    Q_D(QCoapInternalReply);
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::Block2) {
        //! TODO Cover with tests
        quint32 blockNumber = 0;
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d->blockSize = static_cast<uint>(1u << ((optionData[option.length()-1] & 0x7) + 4));
    }

    d->message.addOption(option);
}

/*!
    \internal
    Returns the number of the next block if it is not the last block.
    If it is the last block, it returns -1.
*/
int QCoapInternalReply::wantNextBlock()
{
    Q_D(QCoapInternalReply);

    QCoapOption option = d->message.findOptionByName(QCoapOption::Block1);
    if (option.name() != QCoapOption::Invalid) {
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());

        bool hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);

        if (hasNextBlock) {
            quint32 blockNumber = 0;
            for (int i = 0; i < option.length() - 1; ++i)
                blockNumber = (blockNumber << 8) | optionData[i];
            blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
            return static_cast<int>(blockNumber) + 1;
        }
    }

    return -1;
}

/*!
    \internal
    Returns the status code of the reply.
*/
QtCoap::StatusCode QCoapInternalReply::statusCode() const
{
    Q_D(const QCoapInternalReply);
    return d->statusCode;
}

QT_END_NAMESPACE
