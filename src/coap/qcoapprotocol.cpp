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

/*!
    Creates and sets up a new QCoapInternalRequest related to the request
    associated to the \a reply. The request will then be sent to the server
    using the given \a connection.
*/
void QCoapProtocol::sendRequest(QPointer<QCoapReply> reply, QCoapConnection *connection)
{
    Q_D(QCoapProtocol);

    if (reply.isNull())
        return;

    // Generate unique token and message id
    QCoapInternalRequest *internalRequest = new QCoapInternalRequest(reply->request(), this);
    if (internalRequest->message()->messageId() == 0) {
        do {
            internalRequest->generateMessageId();
        } while (d->containsMessageId(internalRequest->message()->messageId()));
    }
    if (internalRequest->message()->token().isEmpty()) {
        do {
            internalRequest->generateToken();
        } while (d->containsToken(internalRequest->message()->token()));
    }

    internalRequest->setConnection(connection);

    // If this request does not already exist we add it to the map
    if (!reply.isNull() && !d->findInternalRequestByToken(internalRequest->message()->token())) {
        InternalMessagePair pair = { reply, QList<QCoapInternalReply*>() };
        d->internalReplies[internalRequest] = pair;
        reply->setIsRunning(true);
    }

    // If the user specified a size for blockwise request/replies
    if (d->blockSize > 0) {
        internalRequest->setRequestToAskBlock(0, d->blockSize);
        if (internalRequest->message()->payload().length() > d->blockSize)
            internalRequest->setRequestToSendBlock(0, d->blockSize);
    }

    if (internalRequest->message()->type() == QCoapMessage::Confirmable) {
        internalRequest->setTimeout(QRandomGenerator::bounded(d->ackTimeout, static_cast<uint>(d->ackTimeout * d->ackRandomFactor)));
        connect(internalRequest, SIGNAL(timeout(QCoapInternalRequest*)),
                this, SLOT(resendRequest(QCoapInternalRequest*)));
    }

    QMetaObject::invokeMethod(this, "sendRequest",
                              Q_ARG(QCoapInternalRequest*, internalRequest));
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Encodes and sends the given \a request to the server.
*/
void QCoapProtocolPrivate::sendRequest(QCoapInternalRequest *request)
{
    request->beginTransmission();
    QByteArray requestFrame = encode(request);
    QUrl uri = request->targetUri();
    request->connection()->sendRequest(requestFrame, uri.host(), uri.port());
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Handles what to do when a new message is received.

    Here, it puts the frame \a frameReply into a queue and runs the function
    that handles the frames if it is the first in the queue.
*/
void QCoapProtocolPrivate::messageReceived(const QByteArray &frameReply)
{
    frameQueue.enqueue(frameReply);

    if (frameQueue.size() == 1) {
        do {
            handleFrame(frameQueue.head());
            frameQueue.dequeue();
            // Continue until queue is empty
            // TODO I don't think this is possible anymore, possible
            // code simplification.
        } while (frameQueue.size() > 0);
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    This slot is used to send again the given \a request after a timeout or
    aborts the request and transfers a timeout error to the reply.
*/
void QCoapProtocolPrivate::resendRequest(QCoapInternalRequest *request)
{
    // In case of retransmission, check if it is not the last try
    if (request->message()->type() == QCoapMessage::Confirmable
        && internalReplies.contains(request)) {
        if (request->retransmissionCounter() < maxRetransmit) {
            sendRequest(request);
        } else {
            internalReplies[request].userReply->setError(QCoapReply::TimeOutError);
            internalReplies[request].userReply->abortRequest();
        }
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Handles the given \a frame and takes the next.
*/
void QCoapProtocolPrivate::handleFrame(const QByteArray &frame)
{
    QCoapInternalReply *internalReply = decode(frame);
    QCoapInternalRequest *request = nullptr;
    const QCoapMessage *internalReplyMessage = internalReply->message();

    if (!internalReplyMessage->token().isEmpty())
        request = findInternalRequestByToken(internalReplyMessage->token());

    if (!request) {
        request = findInternalRequestByMessageId(internalReplyMessage->messageId());

        // No matching request found, drop the frame.
        if (!request) {
            delete internalReply;
            return;
        }
    }

    request->stopTransmission();
    internalReplies[request].replies.push_back(internalReply);

    // Reply when the server asks for an ACK
    if (request->cancelObserve()) {
        // Remove option to ensure that it will stop
        request->removeOption(QCoapOption::Observe);
        sendReset(request);
    } else if (internalReplyMessage->type() == QCoapMessage::Confirmable) {
        sendAcknowledgement(request);
    }

    // Check if it is a blockwise request
    int nextBlockWanted = internalReply->wantNextBlock();

    // Ask/Send next block or process the final reply
    if (internalReply->hasNextBlock()) {
        onNextBlock(request, internalReply->currentBlockNumber(), internalReply->blockSize());
    } else if (nextBlockWanted >= 0) {
        request->setRequestToSendBlock(static_cast<uint>(nextBlockWanted), blockSize);
        sendRequest(request);
    } else {
        onLastBlock(request);
    }
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Finds an internal request containing the given \a token.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByToken(const QByteArray &token)
{
    for (InternalMessageMap::iterator it = internalReplies.begin();
        it != internalReplies.end(); ++it) {
        if (it.key()->message()->token() == token)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Finds an internal request matching the given \a reply.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByReply(const QCoapReply *reply)
{
    for (InternalMessageMap::iterator it = internalReplies.begin(); it != internalReplies.end(); ++it) {
        if (it.value().userReply == reply)
            return const_cast<QCoapInternalRequest*>(it.key());
    }

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Finds an internal request containing the message id \a messageId.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByMessageId(quint16 messageId)
{
    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message()->messageId() == messageId)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Handles what to do when we received the last block of a reply.

    Merges all blocks, removes the request from the map, updates the
    associated QCoapReply and emits a
    \l{QCoapProtocol::finished(QCoapReply*)}{finished(QCoapReply*)} signal.
*/
void QCoapProtocolPrivate::onLastBlock(QCoapInternalRequest *request)
{
    Q_Q(QCoapProtocol);

    if (!internalReplies.contains(request))
        return;

    QList<QCoapInternalReply*> replies = internalReplies[request].replies;
    QPointer<QCoapReply> userReply = internalReplies[request].userReply;
    if (replies.isEmpty() || userReply.isNull())
        return;

    QCoapInternalReply *finalReply(replies.last());
    if (finalReply->message()->type() == QCoapMessage::Acknowledgement
            && finalReply->statusCode() == QtCoap::Invalid)
        return;

    // If multiple blocks : append data from all blocks to the final reply
    if (replies.size() > 1) {
        std::stable_sort(std::begin(replies), std::end(replies),
            [](QCoapInternalReply *a, QCoapInternalReply *b) -> bool {
                return (a->currentBlockNumber() < b->currentBlockNumber());
        });

        QByteArray finalPayload;
        int lastBlockNumber = -1;
        for (QCoapInternalReply *reply : qAsConst(replies)) {
            int currentBlock = static_cast<int>(reply->currentBlockNumber());
            QByteArray replyPayload = reply->message()->payload();
            if (replyPayload.isEmpty() || currentBlock <= lastBlockNumber)
                continue;

            finalPayload.append(replyPayload);
            lastBlockNumber = currentBlock;
        }

        finalReply->message()->setPayload(finalPayload);
    }

    // Remove the request or the replies
    if ((!userReply.isNull() && !userReply->request().observe()) || request->cancelObserve())
        internalReplies.remove(request);
    else
        internalReplies[request].replies.clear();

    if (!userReply.isNull() && !userReply->isAborted()) {
        userReply->updateFromInternalReply(*finalReply);
        emit q->finished(userReply);
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
void QCoapProtocolPrivate::onNextBlock(QCoapInternalRequest *request,
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
void QCoapProtocolPrivate::sendAcknowledgement(QCoapInternalRequest *request)
{
    QCoapInternalRequest ackRequest;
    QCoapInternalReply *internalReply = internalReplies[request].replies.last();

    ackRequest.setTargetUri(request->targetUri());
    ackRequest.initForAcknowledgement(internalReply->message()->messageId(),
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
    QCoapInternalRequest resetRequest;
    QCoapInternalReply *internalReply = internalReplies[request].replies.last();

    resetRequest.setTargetUri(request->targetUri());
    resetRequest.initForReset(internalReply->message()->messageId());
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
    QCoapInternalRequest *copyRequest = d->findInternalRequestByReply(reply);
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

    Decodes the QByteArray \a message to a QCoapInternalReply object.
*/
QCoapInternalReply *QCoapProtocolPrivate::decode(const QByteArray &message)
{
    Q_Q(QCoapProtocol);
    return new QCoapInternalReply(QCoapInternalReply::fromQByteArray(message), q);
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Aborts the request corresponding to the given \a reply. This is triggered
    by the destruction of the QCoapReply object or a call to
    QCoapReply::abortRequest().
*/
void QCoapProtocolPrivate::onAbortedRequest(const QCoapReply *reply)
{
    QCoapInternalRequest *request = findInternalRequestByReply(reply);
    if (request) {
        request->stopTransmission();
        internalReplies.remove(request);
    }
}

/*!
    Decodes the QByteArray \a data to a list of QCoapResource objects.
    The \a data byte array is a frame returned by a discovery request.
*/
QList<QCoapResource> QCoapProtocol::resourcesFromCoreLinkList(const QByteArray &data)
{
    QList<QCoapResource> resourceList;

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

    Returns true if a request has a token equal to \a token.
*/
bool QCoapProtocolPrivate::containsToken(const QByteArray &token)
{
    for (QCoapInternalRequest *request : internalReplies.keys()) {
        if (request->message()->token() == token)
            return true;
    }

    return false;
}

/*!
    \internal
    \class QCoapProtocolPrivate

    Returns true if a request has a message id equal to \a id.
*/
bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (QCoapInternalRequest *request : internalReplies.keys()) {
        if (request->message()->messageId() == id)
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
    acknowledgement or reset.
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
    Sets the ACK_TIMEOUT value to \a ackTimeout in milliseconds. This value
    defauts to 2000 ms.

    Timeout only applies to Confirmable message. The actual timeout for
    reliable transmissions is a random value between ackTimeout() and
    ackTimeout() * ackRandomFactor().

    \sa ackTimeout(), setAckRandomFactor()
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
