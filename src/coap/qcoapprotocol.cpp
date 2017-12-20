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

#include <QtCore/qrandom.h>
#include <QtCore/qthread.h>
#include "qcoapprotocol_p.h"
#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCoapProtocol
    \brief The QCoapProtocol class handles the logical part of the CoAP
    protocol.

    \reentrant

    The QCoapProtocol is used by the QCoapClient class to handle the logical
    part of the protocol. It can encode requests and decode replies. It also
    handles what to do when a message is received, along with retransmission of
    lost messages.

    \sa QCoapClient
*/

/*!
    Constructs a new QCoapProtocol and sets \a parent as the parent object.
*/
QCoapProtocol::QCoapProtocol(QObject *parent) :
    QObject(* new QCoapProtocolPrivate, parent)
{
    qRegisterMetaType<QCoapInternalRequest*>();
}

QCoapProtocol::~QCoapProtocol()
{
    Q_D(QCoapProtocol);

    // Clear table to avoid double deletion from QObject parenting and QSharedPointer.
    d->exchangeMap.clear();
}

/*!
    Creates and sets up a new QCoapInternalRequest related to the request
    associated to the \a reply. The request will then be sent to the server
    using the given \a connection.
*/
void QCoapProtocol::sendRequest(QPointer<QCoapReply> reply, QCoapConnection *connection)
{
    Q_D(QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == thread());

    if (reply.isNull() || !reply->request().isValid())
        return;

    QSharedPointer<QCoapInternalRequest> internalRequest = QSharedPointer<QCoapInternalRequest>::create(reply->request(), this);
    QCoapMessage *requestMessage = internalRequest->message();
    connect(reply, &QCoapReply::finished, this, &QCoapProtocol::finished);

    // Find a unique Message Id
    if (requestMessage->messageId() == 0) {
        do {
            internalRequest->generateMessageId();
        } while (d->isMessageIdRegistered(requestMessage->messageId()));
    }

    // Find a unique Token
    if (requestMessage->token().isEmpty()) {
        do {
            internalRequest->generateToken();
        } while (d->isTokenRegistered(requestMessage->token()));
    }

    internalRequest->setConnection(connection);

    d->registerExchange(requestMessage->token(), reply, internalRequest);
    reply->setRunning(requestMessage->token(), requestMessage->messageId());

    // If the user specified a size for blockwise request/replies
    if (d->blockSize > 0) {
        internalRequest->setRequestToAskBlock(0, d->blockSize);
        if (requestMessage->payload().length() > d->blockSize)
            internalRequest->setRequestToSendBlock(0, d->blockSize);
    }

    if (requestMessage->type() == QCoapMessage::Confirmable) {
        internalRequest->setTimeout(
                    QtCoap::randomGenerator.bounded(minTimeout(), maxTimeout()));
        connect(internalRequest.data(), SIGNAL(timeout(QCoapInternalRequest*)),
                this, SLOT(resendRequest(QCoapInternalRequest*)));
    }

    d->sendRequest(internalRequest.data());
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Encodes and sends the given \a request to the server.
*/
void QCoapProtocolPrivate::sendRequest(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    if (!request || !request->connection()) {
        qWarning() << "QtCoap: Request null or not bound to any connection: aborted.";
        return;
    }

    request->beginTransmission();
    QByteArray requestFrame = encode(request);
    QUrl uri = request->targetUri();
    request->connection()->sendRequest(requestFrame, uri.host(), uri.port());
}

/*!
    \internal
    \class QCoapProtocolPrivate

    This slot is used to send again the given \a request after a timeout or
    aborts the request and transfers a timeout error to the reply.
*/
void QCoapProtocolPrivate::resendRequest(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    // In case of retransmission, check if it is not the last try
    if (request->message()->type() == QCoapMessage::Confirmable
        && isRequestRegistered(request)) {
        if (request->retransmissionCounter() < maxRetransmit) {
            sendRequest(request);
        } else {
            QCoapReply *reply = userReplyForToken(request->token());
            reply->setError(QCoapReply::TimeOutError);
            reply->abortRequest();
        }
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Decode and process the given \a frame after reception.
*/
void QCoapProtocolPrivate::onFrameReceived(const QByteArray &frame)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QSharedPointer<QCoapInternalReply> internalReply(decode(frame));
    const QCoapMessage *internalReplyMessage = internalReply->message();

    QCoapInternalRequest *request = nullptr;
    if (!internalReplyMessage->token().isEmpty())
        request = requestForToken(internalReplyMessage->token());

    if (!request) {
        request = findRequestByMessageId(internalReplyMessage->messageId());

        // No matching request found, drop the frame.
        if (!request) {
            return;
        }
    }

    request->stopTransmission();
    addReply(internalReplyMessage->token(), internalReply);

    // Reply when the server asks for an ACK
    if (request->isObserveCancelled()) {
        // Remove option to ensure that it will stop
        request->removeOption(QCoapOption::Observe);
        sendReset(request);
    } else if (internalReplyMessage->type() == QCoapMessage::Confirmable) {
        sendAcknowledgment(request);
    }

    // Ask/Send next block or process the final reply
    int nextBlockWanted = internalReply->nextBlockWanted();
    if (nextBlockWanted >= 0) {
        request->setRequestToSendBlock(static_cast<uint>(nextBlockWanted), blockSize);
        sendRequest(request);
    } else if (internalReply->hasNextBlock()) {
        onBlockReceived(request, internalReply->currentBlockNumber(), internalReply->blockSize());
    } else {
        onLastMessageReceived(request);
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns the internal request for the given \a token.
*/
QCoapInternalRequest *QCoapProtocolPrivate::requestForToken(const QByteArray &token)
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return (*it).request.data();

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns the QCoapReply instance of the given \a token.
*/
QPointer<QCoapReply> QCoapProtocolPrivate::userReplyForToken(const QCoapToken &token)
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return (*it).userReply;

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns the replies for the exchange identified by \a token.
*/
QVector<QSharedPointer<QCoapInternalReply> > QCoapProtocolPrivate::repliesForToken(const QCoapToken &token)
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return (*it).replies;

    return QVector<QSharedPointer<QCoapInternalReply> >();
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns the last reply for the exchange identified by \a token.
*/
QCoapInternalReply *QCoapProtocolPrivate::lastReplyForToken(const QCoapToken &token)
{
    auto it = exchangeMap.find(token);
    if (it != exchangeMap.constEnd())
        return (*it).replies.last().data();

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Finds an internal request matching the given \a reply.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findRequestByUserReply(const QCoapReply *reply)
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if ((*it).userReply == reply)
            return (*it).request.data();
    }

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Finds an internal request containing the message id \a messageId.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findRequestByMessageId(quint16 messageId)
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if ((*it).request->message()->messageId() == messageId)
            return (*it).request.data();
    }

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Handles what to do when we received the last block of a reply.

    Merges all blocks, removes the request from the map, updates the
    associated QCoapReply and emits the
    \l{QCoapProtocol::finished(QCoapReply*)}{finished(QCoapReply*)} signal.
*/
void QCoapProtocolPrivate::onLastMessageReceived(QCoapInternalRequest *request)
{
    if (!request || !isRequestRegistered(request))
        return;

    auto replies = repliesForToken(request->token());
    QPointer<QCoapReply> userReply = userReplyForToken(request->token());

    //! FIXME: Change QPointer<QCoapReply> into something independent from
    //! User. QSharedPointer(s)?
    if (replies.isEmpty() || userReply.isNull()) {
        forgetExchange(request->token());
        return;
    }

    auto finalInternalReply = replies.last();
    // Ignore empty ACK messages
    if (finalInternalReply->message()->type() == QCoapMessage::Acknowledgment
            && finalInternalReply->statusCode() == QtCoap::EmptyMessage) {
        exchangeMap[request->token()].replies.takeLast();
        return;
    }

    // Merge payloads for blockwise transfers
    if (replies.size() > 1) {
        std::stable_sort(std::begin(replies), std::end(replies),
            [](auto a, auto b) -> bool {
                return (a->currentBlockNumber() < b->currentBlockNumber());
        });

        QByteArray finalPayload;
        int lastBlockNumber = -1;
        for (auto reply : qAsConst(replies)) {
            int currentBlock = static_cast<int>(reply->currentBlockNumber());
            QByteArray replyPayload = reply->message()->payload();
            if (replyPayload.isEmpty() || currentBlock <= lastBlockNumber)
                continue;

            finalPayload.append(replyPayload);
            lastBlockNumber = currentBlock;
        }

        finalInternalReply->message()->setPayload(finalPayload);
    }

    if (!userReply.isNull())
        userReply->onReplyReceived(finalInternalReply.data());

    // Remove request and replies. Keep the request only for Observe.
    if (userReply.isNull() || !userReply->request().isObserved() || request->isObserveCancelled()) {
        forgetExchange(request->token());
    } else {
        forgetExchangeReplies(request->token());
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Handles what to do when we received a new block that is not the last.

    Here it sets the given internal \a request to ask the block number
    that follow \a currentBlockNumber with a size of \a blockSize
    and sends this new request.
*/
void QCoapProtocolPrivate::onBlockReceived(QCoapInternalRequest *request,
                                       uint currentBlockNumber,
                                       uint blockSize)
{
    request->setRequestToAskBlock(currentBlockNumber + 1, blockSize);
    sendRequest(request);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Sends an internal request acknowledging the given \a request, reusing its
    URI and connection.
*/
void QCoapProtocolPrivate::sendAcknowledgment(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QCoapInternalRequest ackRequest;
    ackRequest.setTargetUri(request->targetUri());

    auto internalReply = lastReplyForToken(request->token());
    ackRequest.initForAcknowledgment(internalReply->message()->messageId(),
                                     internalReply->message()->token());
    ackRequest.setConnection(request->connection());
    sendRequest(&ackRequest);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Sends a Reset message (RST), reusing the details of the given
    \a request. A Reset message indicates that a specific message has been
    received, but cannot be properly processed.
*/
void QCoapProtocolPrivate::sendReset(QCoapInternalRequest *request)
{
    Q_Q(const QCoapProtocol);
    Q_ASSERT(QThread::currentThread() == q->thread());

    QCoapInternalRequest resetRequest;
    resetRequest.setTargetUri(request->targetUri());

    auto lastReply = lastReplyForToken(request->token());
    resetRequest.initForReset(lastReply->message()->messageId());
    resetRequest.setConnection(request->connection());
    sendRequest(&resetRequest);
}

/*!
    Handles what to do when the user want to stop observing a resource.

    Finds the internal request associated with \a reply and tells it to stop
    observing.
*/
void QCoapProtocol::cancelObserve(QPointer<QCoapReply> reply)
{
    if (!reply)
        return;

    Q_D(QCoapProtocol);
    QCoapInternalRequest *copyRequest = d->findRequestByUserReply(reply);
    if (copyRequest)
        copyRequest->setCancelObserve(true);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Encodes the QCoapInternalRequest object \a request to a QByteArray frame.
*/
QByteArray QCoapProtocolPrivate::encode(QCoapInternalRequest *request)
{
    return request->toQByteArray();
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Decodes the QByteArray \a message and returns a new unmanaged
    QCoapInternalReply object.
*/
QCoapInternalReply *QCoapProtocolPrivate::decode(const QByteArray &message)
{
    Q_Q(QCoapProtocol);
    return new QCoapInternalReply(QCoapInternalReply::fromQByteArray(message), q);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Aborts the request corresponding to the given \a reply. It is triggered
    by the destruction of the QCoapReply object or a call to
    QCoapReply::abortRequest().
*/
void QCoapProtocolPrivate::onRequestAborted(const QCoapToken &token)
{
    QCoapInternalRequest *request = requestForToken(token);
    if (!request)
        return;

    request->stopTransmission();
    forgetExchange(request->token());
}

/*!
    Decodes the QByteArray \a data to a list of QCoapResource objects.
    The \a data byte array is a frame returned by a discovery request.
*/
QVector<QCoapResource> QCoapProtocol::resourcesFromCoreLinkList(const QByteArray &data)
{
    QVector<QCoapResource> resourceList;

    QLatin1String quote = QLatin1String("\"");
    const QList<QByteArray> links = data.split(',');
    for (QByteArray link : links)
    {
        QCoapResource resource;
        const QList<QByteArray> parameterList = link.split(';');
        for (QByteArray parameter : parameterList)
        {
            QString parameterString = QString::fromUtf8(parameter);
            int length = parameterString.length();
            if (parameter.startsWith('<'))
                resource.setPath(parameterString.mid(1, length - 2));
            else if (parameter.startsWith("title="))
                resource.setTitle(parameterString.mid(6).remove(quote));
            else if (parameter.startsWith("rt="))
                resource.setResourceType(parameterString.mid(3).remove(quote));
            else if (parameter.startsWith("if="))
                resource.setInterface(parameterString.mid(3).remove(quote));
            else if (parameter.startsWith("sz="))
                resource.setMaximumSize(parameterString.mid(3).remove(quote).toInt());
            else if (parameter.startsWith("ct="))
                resource.setContentFormat(parameterString.mid(3).remove(quote).toUInt());
            else if (parameter == "obs")
                resource.setObservable(true);
        }

        if (!resource.path().isEmpty())
            resourceList.push_back(resource);
    }

    return resourceList;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Registers a new CoAP exchange using \a token.
*/
void QCoapProtocolPrivate::registerExchange(const QCoapToken &token, QCoapReply *reply,
                                            QSharedPointer<QCoapInternalRequest> request)
{
    CoapExchangeData data = { reply, request,
                              QVector<QSharedPointer<QCoapInternalReply> >() };

    exchangeMap.insert(token, data);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Adds \a reply to the list of replies of the exchange identified by
    \a token.
*/
bool QCoapProtocolPrivate::addReply(const QCoapToken &token, QSharedPointer<QCoapInternalReply> reply)
{
    if (!isTokenRegistered(token) || !reply) {
        qWarning() << "QtCoap: Reply token '" << token << "' not registered, or reply is null.";
        return false;
    }

    exchangeMap[token].replies.push_back(reply);
    return true;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Remove the exchange, typically done when finished or aborted.
    This will delete the QCoapInternalRequest and QCoapInternalReplies
    associated with it.
*/
bool QCoapProtocolPrivate::forgetExchange(const QCoapToken &token)
{
    return (exchangeMap.remove(token) > 0);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Remove all replies for the exchange corresponding to \a token.
*/
bool QCoapProtocolPrivate::forgetExchangeReplies(const QCoapToken &token)
{
    auto it = exchangeMap.find(token);
    if (it == exchangeMap.end())
        return false;

    (*it).replies.clear();
    return true;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns true if a request has a token equal to \a token.
*/
bool QCoapProtocolPrivate::isTokenRegistered(const QCoapToken &token)
{
    return exchangeMap.contains(token);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns true the \a request is present in currently registered exchanges.
*/
bool QCoapProtocolPrivate::isRequestRegistered(const QCoapInternalRequest *request)
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if ((*it).request.data() == request)
            return true;
    }

    return false;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns true if a request has a message id equal to \a id.
*/
bool QCoapProtocolPrivate::isMessageIdRegistered(quint16 id)
{
    for (auto it = exchangeMap.constBegin(); it != exchangeMap.constEnd(); ++it) {
        if ((*it).request->message()->messageId() == id)
            return true;
    }

    return false;
}

/*!
    Returns the ACK_TIMEOUT value.

    \sa setAckTimeout()
*/
uint QCoapProtocol::ackTimeout() const
{
    Q_D(const QCoapProtocol);
    return d->ackTimeout;
}

/*!
    Returns the ACK_RANDOM_FACTOR value.

    \sa setAckRandomFactor()
*/
double QCoapProtocol::ackRandomFactor() const
{
    Q_D(const QCoapProtocol);
    return d->ackRandomFactor;
}

/*!
    Returns the MAX_RETRANSMIT value.

    \sa setMaxRetransmit()
*/
uint QCoapProtocol::maxRetransmit() const
{
    Q_D(const QCoapProtocol);
    return d->maxRetransmit;
}

/*!
    Returns the max block size wanted.

    \sa setBlockSize()
*/
quint16 QCoapProtocol::blockSize() const
{
    Q_D(const QCoapProtocol);
    return d->blockSize;
}

/*!
    Returns the MAX_RETRANSMIT_SPAN in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}.

    It is the maximum time from the first transmission of a Confirmable
    message to its last retransmission.
*/
uint QCoapProtocol::maxRetransmitSpan() const
{
    if (maxRetransmit() == 0)
        return 0;

    return static_cast<uint>(ackTimeout() * (1u << (maxRetransmit() - 1)) * ackRandomFactor());
}

/*!
    Returns the MAX_RETRANSMIT_WAIT in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}.

    It is the maximum time from the first transmission of a Confirmable
    message to the time when the sender gives up on receiving an
    acknowledgment or reset.
*/
uint QCoapProtocol::maxRetransmitWait() const
{
    return static_cast<uint>(ackTimeout() * (1u << (maxRetransmit() + 1)) * ackRandomFactor());
}

/*!
    Returns the MAX_LATENCY in milliseconds, as defined in
    \l{https://tools.ietf.org/search/rfc7252#section-4.8.2}{RFC 7252}. This
    value is arbitrarily set to 100 seconds by the standard.

    It is the maximum time a datagram is expected to take from the start of
    its transmission to the completion of its reception.
*/
constexpr uint QCoapProtocol::maxLatency()
{
    return 100 * 1000;
}

/*!
    Returns the minimum duration for messages timeout. The timeout is defined
    as a random value between minTimeout() and maxTimeout().

    \sa minTimeout(), setAckTimeout()
*/
uint QCoapProtocol::minTimeout() const {
    Q_D(const QCoapProtocol);
    return d->ackTimeout;
}

/*!
    Returns the maximum duration for messages timeout.

    \sa maxTimeout(), setAckTimeout(), setAckRandomFactor()
*/
uint QCoapProtocol::maxTimeout() const {
    Q_D(const QCoapProtocol);
    return static_cast<uint>(d->ackTimeout * d->ackRandomFactor);
}

/*!
    Sets the ACK_TIMEOUT value to \a ackTimeout in milliseconds. This value
    defauts to 2000 ms.

    Timeout only applies to Confirmable message. The actual timeout for
    reliable transmissions is a random value between ackTimeout() and
    ackTimeout() * ackRandomFactor().

    \sa ackTimeout(), setAckRandomFactor(), minTimeout(), maxTimeout()
*/
void QCoapProtocol::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapProtocol);
    d->ackTimeout = ackTimeout;
}

/*!
    Sets the ACK_RANDOM_FACTOR value to \a ackRandomFactor. This value
    defaults to 1.5.

    \sa ackRandomFactor(), setAckTimeout()
*/
void QCoapProtocol::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapProtocol);
    d->ackRandomFactor = ackRandomFactor;
}

/*!
    Sets the MAX_RETRANSMIT value to \a maxRetransmit. This value
    defaults to 4.

    \sa maxRetransmit()
*/
void QCoapProtocol::setMaxRetransmit(uint maxRetransmit)
{
    Q_D(QCoapProtocol);
    d->maxRetransmit = maxRetransmit;
}

/*!
    Sets the max block size wanted to \a blockSize.

    \sa blockSize()
*/
void QCoapProtocol::setBlockSize(quint16 blockSize)
{
    // A size of 0 invites the server to chose the block size.
    Q_D(QCoapProtocol);
    d->blockSize = blockSize;
}

QT_END_NAMESPACE

#include "moc_qcoapprotocol.cpp"
