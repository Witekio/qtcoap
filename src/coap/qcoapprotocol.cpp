/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:LGPL3$
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

#include "qcoapprotocol_p.h"
#include "qcoapinternalrequest_p.h"
#include "qcoapinternalreply_p.h"

QT_BEGIN_NAMESPACE

QCoapProtocolPrivate::QCoapProtocolPrivate() :
    blockSize(0),
    ackTimeout(2000),
    ackRandomFactor(1.5),
    maxRetransmit(4)
{
}

/*!
    \class QCoapProtocol
    \brief The QCoapProtocol class handles the logical part of the CoAP
    protocol.

    \reentrant

    The QCoapProtocol is used by the QCoapClient class to handle the logical
    part of the protocol. It can encode requests and decode replies. It also
    handles what to do when a message is received and the retransmission of
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
    QCoapInternalRequest *copyInternalRequest = new QCoapInternalRequest(reply->request(), this);
    if (copyInternalRequest->message().messageId() == 0) {
        do {
            copyInternalRequest->generateMessageId();
        } while (d->containsMessageId(copyInternalRequest->message().messageId()));
    }
    if (copyInternalRequest->message().token().isEmpty()) {
        do {
            copyInternalRequest->generateToken();
        } while (d->containsToken(copyInternalRequest->message().token()));
    }

    copyInternalRequest->setConnection(connection);

    // If this request does not already exist we add it to the map
    if (!reply.isNull() && !d->findInternalRequestByToken(copyInternalRequest->message().token())) {
            InternalMessagePair pair = { reply, QList<QCoapInternalReply*>() };
            d->internalReplies[copyInternalRequest] = pair;
            reply->setIsRunning(true);
    }

    // If the user specified a size for blockwise request/replies
    if (d->blockSize > 0) {
        copyInternalRequest->setRequestToAskBlock(0, d->blockSize);
        if (copyInternalRequest->message().payload().length() > d->blockSize)
            copyInternalRequest->setRequestToSendBlock(0, d->blockSize);
    }

    if (copyInternalRequest->message().type() == QCoapMessage::Confirmable) {
        copyInternalRequest->setTimeout(d->ackTimeout);
        connect(copyInternalRequest, SIGNAL(timeout(QCoapInternalRequest*)),
                this, SLOT(resendRequest(QCoapInternalRequest*)));
    }

    QMetaObject::invokeMethod(this, "sendRequest",
                              Q_ARG(QCoapInternalRequest*, copyInternalRequest));
}

/*!
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
    Handles what to do when a new message is received.

    Here, it puts the frame \a frameReply into a queue and runs the function
    that handles the frames if it is the first in the queue.
*/
void QCoapProtocolPrivate::messageReceived(const QByteArray &frameReply)
{
    frameQueue.enqueue(frameReply);
    if (frameQueue.size() == 1)
        handleFrame();
}

/*!
    Takes the first frame of the queue (without removing it)
    and handles it.
*/
void QCoapProtocolPrivate::handleFrame()
{
    handleFrame(frameQueue.head());
}

/*!
    This slot is used to send again the given \a request after a timeout or
    aborts the request and transfers a timeout error to the reply.
*/
void QCoapProtocolPrivate::resendRequest(QCoapInternalRequest *request)
{
    // In case of retransmission, check if it is not the last try
    if (request->message().type() == QCoapMessage::Confirmable
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
    Handles the given \a frame and takes the next.
*/
void QCoapProtocolPrivate::handleFrame(const QByteArray &frame)
{
    QCoapInternalReply *internalReply = decode(frame);
    QCoapInternalRequest *request = nullptr;
    QCoapMessage internalReplyMessage = internalReply->message();

    if (!internalReplyMessage.token().isEmpty())
        request = findInternalRequestByToken(internalReplyMessage.token());

    if (!request) {
        request = findInternalRequestByMessageId(internalReplyMessage.messageId());
        if (!request)
            return;
    }

    request->stopTransmission();
    internalReplies[request].replies.push_back(internalReply);

    // Reply when the server ask an ACK
    if (request->cancelObserve()) {
        // Remove option to ensure that it will stop
        request->removeOptionByName(QCoapOption::Observe);
        sendReset(request);
    } else if (internalReplyMessage.type() == QCoapMessage::Confirmable) {
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

    // Take the next frame if needed
    frameQueue.dequeue();
    if (!frameQueue.isEmpty())
        handleFrame();
}

/*!
    Finds an internal request containing the given \a token.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByToken(const QByteArray &token)
{
    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message().token() == token)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    Finds an internal request matching the given \a reply.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByReply(QCoapReply *reply)
{
    QCoapInternalRequest *copyRequest = nullptr;
    InternalMessageMap::Iterator it;
    for (it = internalReplies.begin(); it != internalReplies.end(); ++it) {
        if (it.value().userReply == reply) {
            copyRequest = const_cast<QCoapInternalRequest*>(it.key());
            break;
        }
    }

    return copyRequest;
}

/*!
    Finds an internal request containing the message id \a messageId.
*/
QCoapInternalRequest *QCoapProtocolPrivate::findInternalRequestByMessageId(quint16 messageId)
{
    for (InternalMessageMap::iterator it = internalReplies.begin();
         it != internalReplies.end(); ++it) {
        if (it.key()->message().messageId() == messageId)
            return const_cast<QCoapInternalRequest*>(it.key()); // key is the internal request
    }

    return nullptr;
}

/*!
    Handles what to do when we received the last block of a reply.

    Here it merges all blocks, removes the request from the map,
    updates the associated QCoapReply and emits a
    \l{QCoapProtocol::finished(QCoapReply*)}{finished(QCoapReply*)} signal.
*/
void QCoapProtocolPrivate::onLastBlock(QCoapInternalRequest *request)
{
    Q_Q(QCoapProtocol);

    QList<InternalMessagePair> internalRepliesValue = internalReplies.values(request);
    if (internalRepliesValue.isEmpty())
        return;

    QList<QCoapInternalReply*> replies = internalReplies[request].replies;
    QPointer<QCoapReply> userReply = internalReplies[request].userReply;
    if (replies.isEmpty() || userReply.isNull())
        return;

    QCoapInternalReply *finalReply(replies.last());
    if (finalReply->message().type() == QCoapMessage::Acknowledgement
            && finalReply->statusCode() == QCoapInternalReply::Invalid)
        return;

    // If multiple blocks : append data from all blocks to the final reply
    if (replies.size() > 1) {
        qStableSort(std::begin(replies), std::end(replies),
              [](QCoapInternalReply *a, QCoapInternalReply *b) -> bool {
            return (a->currentBlockNumber() < b->currentBlockNumber());
        });

        QByteArray finalPayload;
        int lastBlockNumber = -1;
        for (QCoapInternalReply *reply : qAsConst(replies)) {
            int currentBlock = static_cast<int>(reply->currentBlockNumber());
            QByteArray replyPayload = reply->message().payload();
            if (replyPayload.isEmpty() || currentBlock <= lastBlockNumber)
                continue;

            finalPayload.append(replyPayload);
            lastBlockNumber = static_cast<int>(reply->currentBlockNumber());
        }

        finalReply->message().setPayload(finalPayload);
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
    Handles what to do when we received a new block that is not the last.

    Here it sets the given internal \a request to ask the block number
    that follow \a currentBlockNumber with a size of \a blockSize
    and sends this new request.
*/
void QCoapProtocolPrivate::onNextBlock(QCoapInternalRequest *request,
                                       uint currentBlockNumber,
                                       uint blockSize)
{
    request->setRequestToAskBlock(currentBlockNumber+1, blockSize);
    sendRequest(request);
}

/*!
    Creates a new internal request with the same uri and connection than
    \a request, sets it up to make it an acknowledgement message and sends it.
*/
void QCoapProtocolPrivate::sendAcknowledgement(QCoapInternalRequest *request)
{
    QCoapInternalRequest ackRequest;
    QCoapInternalReply *internalReply = internalReplies[request].replies.last();

    ackRequest.setTargetUri(request->targetUri());
    ackRequest.initForAcknowledgement(internalReply->message().messageId(),
                                     internalReply->message().token());
    ackRequest.setConnection(request->connection());
    sendRequest(&ackRequest);
}

/*!
    Creates a new internal request with the same uri and connection than
    \a request, sets it up to make it a reset message and sends it.
*/
void QCoapProtocolPrivate::sendReset(QCoapInternalRequest *request)
{
    QCoapInternalRequest resetRequest;
    QCoapInternalReply *internalReply = internalReplies[request].replies.last();

    resetRequest.setTargetUri(request->targetUri());
    resetRequest.initForReset(internalReply->message().messageId());
    resetRequest.setConnection(request->connection());
    sendRequest(&resetRequest);
}

/*!
    Handles what to do when the user want to stop to observe a resource.

    Here it just finds the internal request associated to the \a reply and
    sets it to ask to cancel the observe request.
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
    Encodes the QCoapInternalRequest object \a request to a QByteArray frame.
*/
QByteArray QCoapProtocolPrivate::encode(QCoapInternalRequest *request)
{
    return request->toQByteArray();
}

/*!
    Decodes the QByteArray \a message to a QCoapInternalReply object.
*/
QCoapInternalReply *QCoapProtocolPrivate::decode(const QByteArray &message)
{
    Q_Q(QCoapProtocol);
    return new QCoapInternalReply(QCoapInternalReply::fromQByteArray(message), q);
}

/*!
    Handles what to do when the request corresponding to the given
    \a reply has been aborted.

    Here it stops the transmission of the request and removes it from the map.
*/
void QCoapProtocolPrivate::onAbortedRequest(QCoapReply *reply)
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

    QList<QByteArray> links = data.split(',');
    for (QByteArray link : links)
    {
        QCoapResource resource;
        QList<QByteArray> parameterList = link.split(';');
        for (QByteArray parameter : qAsConst(parameterList))
        {
            QString parameterString = QString(parameter);
            int length = parameter.length();
            if (parameter.startsWith('<'))
                resource.setPath(parameterString.mid(1, length-2));
            else if (parameter.startsWith("title="))
                resource.setTitle(parameterString.right(length-6).remove("\""));
            else if (parameter.startsWith("rt="))
                resource.setResourceType(parameterString.right(length-3).remove("\""));
            else if (parameter.startsWith("if="))
                resource.setInterface(parameterString.right(length-3).remove("\""));
            else if (parameter.startsWith("sz="))
                resource.setMaximumSize(parameterString.right(length-3).remove("\"").toInt());
            else if (parameter.startsWith("ct="))
                resource.setContentFormat(parameterString.right(length-3).remove("\"").toUInt());
            else if (parameter == "obs")
                resource.setObservable(true);
        }

        if (!resource.path().isEmpty())
            resourceList.push_back(resource);
    }

    return resourceList;
}

/*!
    Returns true if a request has a token equal to \a token.
*/
bool QCoapProtocolPrivate::containsToken(const QByteArray &token)
{
    for (QCoapInternalRequest *request : internalReplies.keys()) {
        if (request->message().token() == token)
            return true;
    }

    return false;
}

/*!
    Returns true if a request has a message id equal to \a id.
*/
bool QCoapProtocolPrivate::containsMessageId(quint16 id)
{
    for (QCoapInternalRequest *request : internalReplies.keys()) {
        if (request->message().messageId() == id)
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
    Sets the ACK_TIMEOUT value to \a ackTimeout.

    \sa ackTimeout()
*/
void QCoapProtocol::setAckTimeout(uint ackTimeout)
{
    Q_D(QCoapProtocol);
    d->ackTimeout = ackTimeout;
}

/*!
    Sets the ACK_RANDOM_FACTOR value to \a ackRandomFactor.

    \sa ackRandomFactor()
*/
void QCoapProtocol::setAckRandomFactor(double ackRandomFactor)
{
    Q_D(QCoapProtocol);
    d->ackRandomFactor = ackRandomFactor;
}

/*!
    Sets the MAX_RETRANSMIT value to \a maxRetransmit.

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
    // A size of 0 mean that the server choose the blocks size
    Q_D(QCoapProtocol);
    d->blockSize = blockSize;
}

QT_END_NAMESPACE

#include "moc_qcoapprotocol.cpp"
