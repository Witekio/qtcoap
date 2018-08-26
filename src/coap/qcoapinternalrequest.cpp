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
#include <QtCore/qregularexpression.h>
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
    d->timeoutTimer = new QTimer(this);
    connect(d->timeoutTimer, SIGNAL(timeout()), this, SLOT(_q_timeout()));

    d->maxTransmitWaitTimer = new QTimer(this);
    connect(d->maxTransmitWaitTimer, SIGNAL(timeout()), this, SLOT(_q_maxTransmissionSpanReached()));
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
    for (const QCoapOption &option : request.options())
        d->message.addOption(option);
    d->message.setPayload(request.payload());
    d->method = request.method();
    d->fullPayload = request.payload();

    addUriOptions(request.url(), request.proxyUrl());
}

/*!
    \internal
    Returns \c true if the request is considered valid.
*/
bool QCoapInternalRequest::isValid() const
{
    Q_D(const QCoapInternalRequest);
    return isUrlValid(d->targetUri) && d->method != QtCoap::Invalid;
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into an
    acknowledgment message with the message id \a messageId and the given
    \a token.
*/
void QCoapInternalRequest::initForAcknowledgment(quint16 messageId, const QByteArray &token)
{
    Q_D(QCoapInternalRequest);

    setMethod(QtCoap::Invalid);
    d->message.setType(QCoapMessage::Acknowledgment);
    d->message.setMessageId(messageId);
    d->message.setToken(token);
    d->message.setPayload(QByteArray());
    d->message.removeAllOptions();
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into a
    Reset message (RST) with the message id \a messageId.

    A Reset message should contain only the \a messageId.
*/
void QCoapInternalRequest::initForReset(quint16 messageId)
{
    Q_D(QCoapInternalRequest);

    setMethod(QtCoap::Invalid);
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
    if (!d->message.options().isEmpty()) {
        // Sort options by ascending order
        QVector<QCoapOption> options = d->message.options();
        std::sort(options.begin(), options.end(),
            [](const QCoapOption &a, const QCoapOption &b) -> bool {
                return a.name() < b.name();
        });

        quint8 lastOptionNumber = 0;
        for (const QCoapOption &option : qAsConst(options)) {
            quint8 optionPdu;

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            quint16 optionLength = static_cast<quint16>(option.length());
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

    \sa blockOption()
*/
void QCoapInternalRequest::setRequestToAskBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    d->message.setMessageId(d->message.messageId() + 1);
    d->message.removeOption(QCoapOption::Block1);
    d->message.removeOption(QCoapOption::Block2);

    addOption(blockOption(QCoapOption::Block2, blockNumber, blockSize));
}

/*!
    \internal
    Initialize blocks parameters and creates the options needed to send the block with
    the number \a blockNumber and with a size of \a blockSize.

    \sa setRequestToAskBlock(), blockOption()
*/
void QCoapInternalRequest::setRequestToSendBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    d->message.setMessageId(d->message.messageId() + 1);
    d->message.setPayload(d->fullPayload.mid(static_cast<int>(blockNumber * blockSize), static_cast<int>(blockSize)));
    d->message.removeOption(QCoapOption::Block1);

    addOption(blockOption(QCoapOption::Block1, blockNumber, blockSize));
}

/*!
    \internal
    Builds and returns a Block option.

    The \a blockSize should range from 16 to 1024 and be a power of 2,
    computed as 2^(SZX + 4), with SZX ranging from 0 to 6. For more details,
    refer to the \l{https://tools.ietf.org/html/rfc7959#section-2.2}{RFC 7959}.
*/
QCoapOption QCoapInternalRequest::blockOption(QCoapOption::OptionName name, uint blockNumber, uint blockSize) const
{
    Q_D(const QCoapInternalRequest);

    //! TODO Cover this in tests
    Q_ASSERT((blockSize & (blockSize - 1)) == 0); // is a power of two
    Q_ASSERT(!(blockSize >> 11)); // blockSize <= 1024

    // NUM field
    quint32 optionData = (blockNumber << 4);

    // SZX field = log2(blockSize - 4)
    optionData |= (blockSize >> 7)
                  ? ((blockSize >> 10) ? 6 : (3 + (blockSize >> 8)))
                  : (blockSize >> 5);

    // M field set when more data is available to send
    if (name == QCoapOption::Block1
            && static_cast<int>((blockNumber * blockSize) + blockSize) < d->fullPayload.length()) {
        optionData |= 8;
    }

    QByteArray optionValue;
    if (optionData > 0xFFFF)
        optionValue.append(static_cast<char>(optionData >> 16));
    if (optionData > 0xFF)
        optionValue.append(static_cast<char>((optionData >> 8) & 0xFF));
    optionValue.append(static_cast<char>(optionData & 0xFF));

    return QCoapOption(name, optionValue);
}

/*!
    \internal
    Generates a new message id.
*/
quint16 QCoapInternalRequest::generateMessageId()
{
    Q_D(QCoapInternalRequest);

    quint16 id = static_cast<quint16>(QtCoap::randomGenerator.bounded(0x10000));
    d->message.setMessageId(id);
    return id;
}

/*!
    \internal
    Generates a new token.
*/
QCoapToken QCoapInternalRequest::generateToken()
{
    Q_D(QCoapInternalRequest);

    // TODO: Allow setting minimum token size as a security setting
    quint8 length = static_cast<quint8>(QtCoap::randomGenerator.bounded(1, 8));

    QByteArray token(length, 0);
    quint8 *tokenData = reinterpret_cast<quint8 *>(token.data());
    for (int i = 0; i < token.size(); ++i)
        tokenData[i] = static_cast<quint8>(QtCoap::randomGenerator.bounded(256));

    d->message.setToken(token);
    return token;
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.
*/
void QCoapInternalRequest::addOption(const QCoapOption &option)
{
    if (option.name() == QCoapOption::Block1)
        setFromDescriptiveBlockOption(option);

    QCoapInternalMessage::addOption(option);
}

/*!
    \internal
    Adds the CoAP options related to the target and proxy with the given \a uri
    and \a proxyUri. Returns \c true upon success, \c false if an error
    occurred.

    Numbers refer to step numbers from CoAP
    \l{RFC 7252}{https://tools.ietf.org/html/rfc7252#section-6.4}.
*/
bool QCoapInternalRequest::addUriOptions(QUrl uri, const QUrl &proxyUri)
{
    Q_D(QCoapInternalRequest);
    // Set to an invalid state
    d->targetUri = QUrl();

    // When using a proxy uri, we SHOULD NOT include Uri-Host/Port/Path/Query
    // options.
    if (!proxyUri.isEmpty()) {
        if (!isUrlValid(proxyUri))
            return false;

        addOption(QCoapOption(QCoapOption::ProxyUri, proxyUri.toString()));
        d->targetUri = proxyUri;
        return true;
    }

    // 1/3/4. Fails if URL is relative, has no 'coap' scheme or has a fragment
    if (!isUrlValid(uri))
        return false;

    // 2. TODO Ensure encoding matches CoAP standard (= no % in options)

    // 5. Add Uri-Host option if not a plain IP
    QCoapOption uriHost = uriHostOption(uri);
    if (uriHost.isValid())
        addOption(uriHost);

    // 6. Set default port
    if (uri.port() == -1)
        uri.setPort(5683);

    // 7. Add port to options if it is not the default port
    if (uri.port() != 5683)
        addOption(QCoapOption::UriPort, static_cast<quint32>(uri.port()));

    // 8. Add path segments to options
    QString path = uri.path();
    const auto listPath = path.splitRef('/');
    for (const QStringRef &pathPart : listPath) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption(QCoapOption::UriPath, pathPart.toString()));
    }

    // 9. Add queries to options
    QString query = uri.query();
    const auto listQuery = query.splitRef('&');
    for (const QStringRef &queryElement : listQuery) {
        if (!queryElement.isEmpty())
            addOption(QCoapOption(QCoapOption::UriQuery, queryElement.toString()));
    }

    d->targetUri = uri;
    return true;
}

/*!
    \internal
    Returns the token of the request.
*/
QCoapToken QCoapInternalRequest::token() const
{
    return message()->token();
}

/*!
    \internal
    Used to mark the transmission as "in progress", when starting or retrying
    to transmit a message. This method manages the retransmission counter,
    the transmission timeout and the exchange timeout.
*/
void QCoapInternalRequest::restartTransmission()
{
    Q_D(QCoapInternalRequest);

    if (!d->transmissionInProgress) {
        d->transmissionInProgress = true;
        d->maxTransmitWaitTimer->start();
    } else {
        d->retransmissionCounter++;
        d->timeout *= 2;
    }

    if (d->timeout > 0)
        d->timeoutTimer->start(d->timeout);
}

/*!
    \internal
    Marks the transmission as not running, after a successful reception, or an
    error. It resets the retranmission count and stop all timeout timers.
*/
void QCoapInternalRequest::stopTransmission()
{
    Q_D(QCoapInternalRequest);
    d->transmissionInProgress = false;
    d->retransmissionCounter = 0;
    d->maxTransmitWaitTimer->stop();
    d->timeoutTimer->stop();
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
    This slot emits a \l{QCoapInternalRequest::maxTransmissionSpanReached(QCoapInternalRequest*)}
    {timeout(QCoapInternalRequest*)} signal.
*/
void QCoapInternalRequestPrivate::_q_maxTransmissionSpanReached()
{
    Q_Q(QCoapInternalRequest);
    emit q->maxTransmissionSpanReached(q);
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
    Returns true if the request is an Observe request.

*/
bool QCoapInternalRequest::isObserve() const
{
    Q_D(const QCoapInternalRequest);
    return d->message.hasOption(QCoapOption::Observe);
}

/*!
    \internal
    Returns true if the observe request needs to be cancelled.

    \sa setCancelObserve()
*/
bool QCoapInternalRequest::isObserveCancelled() const
{
    Q_D(const QCoapInternalRequest);
    return d->observeCancelled;
}

/*!
    \internal
    Returns the value of the retransmission counter.
*/
int QCoapInternalRequest::retransmissionCounter() const
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
    Marks the observe request as cancelled.

    \sa isObserveCancelled()
*/
void QCoapInternalRequest::setObserveCancelled()
{
    Q_D(QCoapInternalRequest);
    d->observeCancelled = true;
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
    d->timeout = static_cast<int>(timeout);
}

/*!
    \internal
    Sets the maximum transmission span for the request. If the request is
    not finished at the end of the transmission span, the request will timeout.
*/
void QCoapInternalRequest::setMaxTransmissionWait(int duration)
{
    Q_D(QCoapInternalRequest);
    d->maxTransmitWaitTimer->setInterval(duration);
}

/*!
    \internal
    Decode the \a uri provided and returns a QCoapOption.
*/
QCoapOption QCoapInternalRequest::uriHostOption(const QUrl &uri) const
{
    QHostAddress address(uri.host());

    // No need for Uri-Host option with an IPv4 or IPv6 address
    if (!address.isNull())
        return QCoapOption();

    return QCoapOption(QCoapOption::UriHost, uri.host());
}

QT_END_NAMESPACE
