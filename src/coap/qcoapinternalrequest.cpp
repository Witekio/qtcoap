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
#include "qcoapparser_p.h"

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
    d->message = request;
    d->method = request.method();
    d->fullPayload = request.payload();

    updateFromDescriptiveBlockOption(message()->option(QCoapOption::Block1));
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
    Initializes block parameters and creates the options needed to request the
    block \a blockNumber with a size of \a blockSize.

    \sa blockOption(), setToSendBlock()
*/
void QCoapInternalRequest::setToRequestBlock(int blockNumber, int blockSize)
{
    Q_D(QCoapInternalRequest);

    if (!checkBlockNumber(blockNumber))
        return;

    d->message.removeOption(QCoapOption::Block1);
    d->message.removeOption(QCoapOption::Block2);

    auto option = d->parser->generateBlockOption(QCoapOption::Block2, static_cast<uint>(blockNumber),
                                                 static_cast<uint>(blockSize), d->fullPayload.length());
    addOption(option);
}

/*!
    \internal
    Initialize blocks parameters and creates the options needed to send the block with
    the number \a blockNumber and with a size of \a blockSize.

    \sa blockOption(), setToRequestBlock()
*/
void QCoapInternalRequest::setToSendBlock(int blockNumber, int blockSize)
{
    Q_D(QCoapInternalRequest);

    if (!checkBlockNumber(blockNumber))
        return;

    d->message.setPayload(d->fullPayload.mid(blockNumber * blockSize, blockSize));
    d->message.removeOption(QCoapOption::Block1);

    auto option = d->parser->generateBlockOption(QCoapOption::Block1, static_cast<uint>(blockNumber),
                                                 static_cast<uint>(blockSize), d->fullPayload.length());
    addOption(option);
}

/*!
    \internal
    Returns \c true if the block number is valid, false otherwise.
    If the block number is not valid, logs a warning message.
*/
bool QCoapInternalRequest::checkBlockNumber(int blockNumber)
{
    if (blockNumber < 0) {
        qWarning() << "QtCoap: Invalid block number" << blockNumber;
        return false;
    }

    if (blockNumber >> 20) {
        qWarning() << "QtCoap: Block number" << blockNumber << "is too large."
                      " It should fit in 20 bits.";
        return false;
    }

    return true;
}

/*!
    \internal
    Sets the request's message id.
*/
void QCoapInternalRequest::setMessageId(quint16 id)
{
    Q_D(QCoapInternalRequest);
    d->message.setMessageId(id);
}

/*!
    \internal
    Sets the request's token.
*/
void QCoapInternalRequest::setToken(const QCoapToken &token)
{
    Q_D(QCoapInternalRequest);
    d->message.setToken(token);
}

/*!
    \internal
    Adds the given CoAP \a option and sets block parameters if needed.
*/
void QCoapInternalRequest::addOption(const QCoapOption &option)
{
    QCoapInternalMessage::addOption(option);

    if (option.name() == QCoapOption::Block1)
        updateFromDescriptiveBlockOption(QCoapOption::Block1);
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
        uri.setPort(QtCoap::DefaultPort);

    // 7. Add port to options if it is not the default port
    if (uri.port() != QtCoap::DefaultPort)
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
    error. It resets the retranmission count and stops all timeout timers.
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
    {timeout(QCoapInternalRequest*)} signal, which gets forwarded to
    QCoapProtocolPrivate::onRequestTimeout().
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
