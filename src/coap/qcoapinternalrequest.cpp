/****************************************************************************
**
** Copyright (C) 2017 Witekio.
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

#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include "qcoapinternalrequest_p.h"
#include "qcoaprequest.h"

QT_BEGIN_NAMESPACE

/*!
    \internal

    \class QCoapInternalRequest
    \brief The QCoapInternalRequest class contains data related to
    a message that needs to be sent.

    \reentrant

    \sa QCoapInternalMessage, QCoapInternalReply
*/

/*!
    \internal
    Constructs a new QCoapInternalRequest object and sets \a parent as
    the parent object.
*/
QCoapInternalRequest::QCoapInternalRequest(QObject *parent) :
    QCoapInternalMessage(*new QCoapInternalRequestPrivate, parent)
{
    Q_D(QCoapInternalRequest);
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(_q_timeout()));
}

/*!
    \internal
    Constructs a new QCoapInternalRequest object with the information of
    \a request and sets \a parent as the parent object.
*/
QCoapInternalRequest::QCoapInternalRequest(const QCoapRequest &request, QObject *parent) :
    QCoapInternalRequest(parent)
{
    Q_D(QCoapInternalRequest);
    d->message.setVersion(request.version());
    d->message.setType(request.type());
    d->message.setMessageId(request.messageId());
    d->message.setToken(request.token());
    foreach (const QCoapOption &option, request.optionList())
        d->message.addOption(option);
    d->message.setPayload(request.payload());
    d->method = request.method();
    d->fullPayload = request.payload();

    addUriOptions(request.url(), request.proxyUrl());
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into an
    acknowledgement message with the message id \a messageId and the given
    \a token.
*/
void QCoapInternalRequest::initForAcknowledgement(quint16 messageId, const QByteArray &token)
{
    Q_D(QCoapInternalRequest);

    setMethod(QtCoap::Empty);
    d->message.setType(QCoapMessage::Acknowledgement);
    d->message.setMessageId(messageId);
    d->message.setToken(token);
    d->message.setPayload(QByteArray());
    d->message.removeAllOptions();
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into a
    Reset message (RST) with the message id \a messageId.

    A Reset message should be empty, and contain the \a messageId.
*/
void QCoapInternalRequest::initForReset(quint16 messageId)
{
    Q_D(QCoapInternalRequest);

    setMethod(QtCoap::Empty);
    d->message.setType(QCoapMessage::Reset);
    d->message.setMessageId(messageId);
    d->message.setToken(QByteArray());
    d->message.setPayload(QByteArray());
    d->message.removeAllOptions();
}

/*!
    \internal
    Returns the CoAP frame corresponding to the QCoapInternalRequest into
    a QByteArray object.

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
QByteArray QCoapInternalRequest::toQByteArray() const
{
    Q_D(const QCoapInternalRequest);
    QByteArray pdu;

    // Insert header
    pdu.append(static_cast<char>((d->message.version()   << 6)              // CoAP version
                               | (d->message.type()      << 4)              // Message type
                               |  d->message.token().length()));            // Token Length
    pdu.append(static_cast<char>( d->method                    & 0xFF));    // Method type
    pdu.append(static_cast<char>((d->message.messageId() >> 8) & 0xFF));    // Message ID
    pdu.append(static_cast<char>( d->message.messageId()       & 0xFF));

    // Insert Token
    pdu.append(d->message.token());

    // Insert Options
    if (!d->message.optionList().isEmpty()) {
        // Sort options by ascending order
        QList<QCoapOption> optionList = d->message.optionList();
        std::sort(optionList.begin(), optionList.end(),
            [](const QCoapOption &a, const QCoapOption &b) -> bool {
                return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (const QCoapOption &option : qAsConst(optionList)) {
            quint8 optionPdu;

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            quint16 optionLength = option.length();
            bool isOptionLengthExtended = false;
            quint8 optionLengthExtended = 0;

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

            optionPdu = static_cast<quint8>((static_cast<quint8>(optionDelta) << 4)         // Option Delta
                                            | (static_cast<quint8>(optionLength) & 0x0F));  // Option Length
            pdu.append(static_cast<char>(optionPdu));
            if (isOptionDeltaExtended)
                pdu.append(static_cast<char>(optionDeltaExtended));     // Option Delta Extended
            if (isOptionLengthExtended)
                pdu.append(static_cast<char>(optionLengthExtended));    // Option Length Extended
            pdu.append(option.value());                                 // Option Value

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!d->message.payload().isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d->message.payload());
    }

    return pdu;
}

/*!
    \internal
    Initializes block parameters and creates the options needed to ask the
    block with the number \a blockNumber and with a size of \a blockSize.

    The \a blockSize should range from 16 to 1024 and be a power of 2,
    computed as 2^(SZX + 4), with SZX ranging from 0 to 6. For more details,
    refer to the \l{https://tools.ietf.org/html/rfc7959#section-2.2}{RFC 7959}.
*/
void QCoapInternalRequest::setRequestToAskBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    //! TODO Cover this in tests
    Q_ASSERT((blockSize & (blockSize - 1)) == 0); // is a power of two
    Q_ASSERT(!(blockSize >> 10)); // blockSize < 1024

    // Set the Block2Option option to get the new block
    // blockSize = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize) - 4);
    QByteArray block2Value;
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>((block2Data >> 8) & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d->message.removeOption(QCoapOption::Block2);
    d->message.removeOption(QCoapOption::Block1);
    addOption(QCoapOption::Block2, block2Value);

    d->message.setMessageId(d->message.messageId() + 1);
}

/*!
    \internal
    Initialize blocks parameters and creates the options needed to send the block with
    the number \a blockNumber and with a size of \a blockSize.

    \sa setRequestToAskBlock()
*/
void QCoapInternalRequest::setRequestToSendBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    //! TODO Cover this in tests
    Q_ASSERT((blockSize & (blockSize - 1)) == 0); // is a power of two
    Q_ASSERT(!(blockSize >> 10)); // blockSize < 1024

    d->message.setPayload(d->fullPayload.mid(blockNumber * blockSize, blockSize));

    // Set the Block2Option option to get the new block
    // size = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize)-4);
    if (static_cast<int>((blockNumber * blockSize) + blockSize) < d->fullPayload.length())
        block2Data |= 8; // Set the "more" flag to 1

    QByteArray block2Value;
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>((block2Data >> 8) & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d->message.removeOption(QCoapOption::Block1);
    addOption(QCoapOption::Block1, block2Value);

    d->message.setMessageId(d->message.messageId() + 1);
}

/*!
    \internal
    Generates a new message id.
*/
quint16 QCoapInternalRequest::generateMessageId()
{
    Q_D(QCoapInternalRequest);
    quint16 id = static_cast<quint16>(QRandomGenerator::bounded(0x10000));
    d->message.setMessageId(id);
    return id;
}

/*!
    \internal
    Generates a new token.
*/
QByteArray QCoapInternalRequest::generateToken()
{
    Q_D(QCoapInternalRequest);

    QByteArray token("");
    quint8 length = static_cast<quint8>(QRandomGenerator::bounded(1, 8));
    token.resize(length);

    quint8 *tokenData = reinterpret_cast<quint8 *>(token.data());
    for (int i = 0; i < token.size(); ++i)
        tokenData[i] = static_cast<quint8>(QRandomGenerator::bounded(256));

    d->message.setToken(token);
    return token;
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.
*/
void QCoapInternalRequest::addOption(const QCoapOption &option)
{
    Q_D(QCoapInternalRequest);
    //! TODO Cover with tests
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::Block1) {
        quint32 blockNumber = 0;
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());
        quint8 lastByte = optionData[option.length() - 1];

        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | (lastByte >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((lastByte & 0x8) == 0x8);
        d->blockSize = static_cast<uint>(1u << ((lastByte & 0x7) + 4));
    }

    d->message.addOption(option);
}

/*!
    \internal
    Adds the CoAP options related to the target and proxy with the given \a uri
    and \a proxyUri.
*/
void QCoapInternalRequest::addUriOptions(const QUrl &uri, const QUrl &proxyUri)
{
    Q_D(QCoapInternalRequest);

    QUrl mainUri;
    if (proxyUri.isEmpty()) {
        mainUri = uri;
    } else {
        // Add proxy options
        mainUri = proxyUri;
        addOption(QCoapOption::ProxyUri, uri.toString().toUtf8());
    }

    QRegExp ipv4Regex(QLatin1String("^([0-9]{1,3}.){3}([0-9]{1,3})$"));
    QString host = mainUri.host();
    if (!ipv4Regex.exactMatch(host))
        addOption(QCoapOption::UriHost, host.toUtf8());

    // Convert port into QCoapOption if it is not the default port
    int port = mainUri.port();
    if (port > 0 && port != 5683)
        addOption(QCoapOption::UriPort, QByteArray::number(port));

    // Convert path into QCoapOptions
    QString path = mainUri.path();
    const auto listPath = path.splitRef('/');
    for (const QStringRef &pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::UriPath, pathPart.toUtf8());
    }

    // Convert query into QCoapOptions
    QString query = mainUri.query();
    const auto listQuery = query.splitRef('&');
    for (const QStringRef &query : listQuery) {
        if (!query.isEmpty())
            addOption(QCoapOption::UriQuery, query.toUtf8());
    }

    d->targetUri = mainUri;
}

/*!
    \internal
    Increments the retransmission counter, updates the timeout and
    starts a timer.
*/
void QCoapInternalRequest::beginTransmission()
{
    Q_D(QCoapInternalRequest);

    if (d->retransmissionCounter > 0)
        d->timeout *= 2;

    d->retransmissionCounter++;
    if (d->timeout > 0)
        d->timer->start(d->timeout);
}

/*!
    \internal
    Resets the retransmission counter to zero and stops the timer.
*/
void QCoapInternalRequest::stopTransmission()
{
    Q_D(QCoapInternalRequest);
    d->retransmissionCounter = 0;
    d->timer->stop();
    d->timer->setInterval(0);
}

/*!
    \internal
    This slot emits a \l{QCoapInternalRequest::timeout(QCoapInternalRequest*)}
    {timeout(QCoapInternalRequest*)} signal.
*/
void QCoapInternalRequestPrivate::_q_timeout()
{
    Q_Q(QCoapInternalRequest);
    emit q->timeout(q);
}

/*!
    \internal
    Returns the target uri.

    \sa setTargetUri()
*/
QUrl QCoapInternalRequest::targetUri() const
{
    Q_D(const QCoapInternalRequest);
    return d->targetUri;
}

/*!
    \internal
    Returns the connection used to send this request.

    \sa setConnection()
*/
QCoapConnection *QCoapInternalRequest::connection() const
{
    Q_D(const QCoapInternalRequest);
    return d->connection;
}

/*!
    \internal
    Returns the method of the request.

    \sa setMethod()
*/
QtCoap::Method QCoapInternalRequest::method() const
{
    Q_D(const QCoapInternalRequest);
    return d->method;
}

/*!
    \internal
    Returns true if the observe request needs to be cancelled.

    \sa setCancelObserve()
*/
bool QCoapInternalRequest::cancelObserve() const
{
    Q_D(const QCoapInternalRequest);
    return d->cancelObserve;
}

/*!
    \internal
    Returns the value of the retransmission counter.
*/
uint QCoapInternalRequest::retransmissionCounter() const
{
    Q_D(const QCoapInternalRequest);
    return d->retransmissionCounter;
}

/*!
    \internal
    Sets the method of the request to the given \a method.

    \sa method()
*/
void QCoapInternalRequest::setMethod(QtCoap::Method method)
{
    Q_D(QCoapInternalRequest);
    d->method = method;
}

/*!
    \internal
    Sets the connection to use to send this request to the given \a connection.

    \sa connection()
*/
void QCoapInternalRequest::setConnection(QCoapConnection *connection)
{
    Q_D(QCoapInternalRequest);
    d->connection = connection;
}

/*!
    \internal
    Sets the cancel observe parameter to the given \a cancelObserve value.

    \sa cancelObserve()
*/
void QCoapInternalRequest::setCancelObserve(bool cancelObserve)
{
    Q_D(QCoapInternalRequest);
    d->cancelObserve = cancelObserve;
}

/*!
    \internal
    Sets the target uri to the given \a targetUri.

    \sa targetUri()
*/
void QCoapInternalRequest::setTargetUri(QUrl targetUri)
{
    Q_D(QCoapInternalRequest);
    d->targetUri = targetUri;
}

/*!
    \internal
    Sets the timeout to the given \a timeout value in milliseconds. Timeout is
    used for reliable transmission of Confirmable messages.

    When such request times out, its timeout value will double.
*/
void QCoapInternalRequest::setTimeout(uint timeout)
{
    Q_D(QCoapInternalRequest);
    d->timeout = timeout;
}

/*!
    \internal
    Returns true if this QCoapInternalRequest has a lower message id than
    \a other has.
*/
bool QCoapInternalRequest::operator<(const QCoapInternalRequest &other) const
{
    Q_D(const QCoapInternalRequest);
    const QCoapInternalRequestPrivate *d_other = other.d_func();

    return (d->message.messageId() < d_other->message.messageId());
}

QT_END_NAMESPACE
