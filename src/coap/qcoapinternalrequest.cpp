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

#include "qcoapinternalrequest_p.h"
#include "qcoaprequest.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

QCoapInternalRequestPrivate::QCoapInternalRequestPrivate() :
    targetUri(QUrl()),
    operation(QCoapInternalRequest::Empty),
    cancelObserve(false),
    retransmissionCounter(0),
    timeout(0),
    timer(nullptr)
{
}

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
    for (int i = 0; i < request.optionsLength(); ++i)
        d->message.addOption(request.option(i));
    d->message.setPayload(request.payload());
    d->operation = QCoapInternalRequest::OperationInternal(request.operation());
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

    setOperation(Empty);
    d->message.setType(QCoapMessage::Acknowledgement);
    d->message.setMessageId(messageId);
    d->message.setToken(token);
    d->message.setPayload(QByteArray());
    d->message.removeAllOptions();
}

/*!
    \internal
    Initialize parameters to transform the QCoapInternalRequest into a
    reset message with the message id \a messageId.
*/
void QCoapInternalRequest::initForReset(quint16 messageId)
{
    Q_D(QCoapInternalRequest);

    setOperation(Empty);
    d->message.setType(QCoapMessage::Reset);
    d->message.setMessageId(messageId);
    d->message.setToken(QByteArray());
    d->message.setPayload(QByteArray());
    d->message.removeAllOptions();
}

/*!
    \internal
    Returns the coap frame corresponding to the QCoapInternalRequest into
    a QByteArray object.
*/
QByteArray QCoapInternalRequest::toQByteArray() const
{
    Q_D(const QCoapInternalRequest);
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(d->message.version()) << 30)              // Coap version
                         | (quint32(d->message.type()) << 28)               // Message type
                         | (quint32(d->message.token().length()) << 24)     // Token Length
                         | (quint32(d->operation) << 16)                    // Operation type
                         | (quint32(d->message.messageId()));               // Message ID

    pdu.append(static_cast<char>(coapHeader >> 24));
    pdu.append(static_cast<char>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<char>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<char>(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d->message.token());

    // Insert Options
    if (!d->message.optionList().isEmpty()) {
        // Sort options by ascending order
        QList<QCoapOption> optionList = d->message.optionList();
        qSort(optionList.begin(), optionList.end(),
              [](const QCoapOption &a, const QCoapOption &b) -> bool {
            return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (QCoapOption option :qAsConst(optionList)) {
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
    Initialize blocks parameters and create the options needed to ask the block with
    the number \a blockNumber and with a size of \a blockSize.
*/
void QCoapInternalRequest::setRequestToAskBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    // Set the Block2Option option to get the new block
    // blockSize = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize) - 4);
    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>(block2Data >> 8) & 0xFF);
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d->message.removeOptionByName(QCoapOption::Block2);
    d->message.removeOptionByName(QCoapOption::Block1);
    addOption(QCoapOption::Block2, block2Value);

    d->message.setMessageId(d->message.messageId() + 1);
}

/*!
    \internal
    Initialize blocks parameters and creates the options needed to send the block with
    the number \a blockNumber and with a size of \a blockSize.
*/
void QCoapInternalRequest::setRequestToSendBlock(uint blockNumber, uint blockSize)
{
    Q_D(QCoapInternalRequest);

    d->message.setPayload(d->fullPayload.mid(blockNumber*blockSize, blockSize));

    // Set the Block2Option option to get the new block
    // size = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize)-4);
    if (static_cast<int>((blockNumber * blockSize) + blockSize) < d->fullPayload.length())
        block2Data |= 8; // Set the "more" flag to 1

    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>(block2Data >> 8 & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d->message.removeOptionByName(QCoapOption::Block1);
    addOption(QCoapOption::Block1, block2Value);

    d->message.setMessageId(d->message.messageId()+1);
}

/*!
    \internal
    Generates a new message id.
*/
quint16 QCoapInternalRequest::generateMessageId()
{
    Q_D(QCoapInternalRequest);
    quint16 id = static_cast<quint16>(qrand() % 65536);
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
    quint8 length = (qrand() % 7) + 1;
    token.resize(length);

    quint8 *tokenData = reinterpret_cast<quint8 *>(token.data());
    for (int i = 0; i < token.size(); ++i)
        tokenData[i] = static_cast<quint8>(qrand() % 256);

    d->message.setToken(token);
    return token;
}

/*!
    \internal
    Adds the given coap \a option and sets block parameters if needed.
*/
void QCoapInternalRequest::addOption(const QCoapOption &option)
{
    Q_D(QCoapInternalRequest);
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::Block1) {
        quint32 blockNumber = 0;
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d->blockSize = static_cast<uint>(qPow(2, (optionData[option.length()-1] & 0x7) + 4));
    }

    d->message.addOption(option);
}

/*!
    \internal
    Adds the coap options related to the target and proxy with the given \a uri
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
        addOption(QCoapOption::UriPort, QByteArray::number(port, 10));

    // Convert path into QCoapOptions
    QString path = mainUri.path();
    QStringList listPath = path.split('/');
    for (QString pathPart : qAsConst(listPath)) {
        if (!pathPart.isEmpty())
            addOption(QCoapOption::UriPath, pathPart.toUtf8());
    }

    // Convert query into QCoapOptions
    QString query = mainUri.query();
    QStringList listQuery = query.split('&');
    for (const QString query : qAsConst(listQuery)) {
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
    Returns the operation type of the request.

    \sa setOperation()
*/
QCoapInternalRequest::OperationInternal QCoapInternalRequest::operation() const
{
    Q_D(const QCoapInternalRequest);
    return d->operation;
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
    Sets the operation type of the request to the given \a operation.

    \sa operation()
*/
void QCoapInternalRequest::setOperation(OperationInternal operation)
{
    Q_D(QCoapInternalRequest);
    d->operation = operation;
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
    Sets the timeout to the given \a timeout value in milliseconds.
*/
void QCoapInternalRequest::setTimeout(uint timeout)
{
    Q_D(QCoapInternalRequest);
    d->timeout = timeout;
}

/*!
    \internal
    Returns true if this QCoapInternalRequest message id is lower than \a other
    message id.
*/
bool QCoapInternalRequest::operator<(const QCoapInternalRequest &other) const
{
    Q_D(const QCoapInternalRequest);
    const QCoapInternalRequestPrivate *d_other = other.d_func();

    return (d->message.messageId() < d_other->message.messageId());
}

QT_END_NAMESPACE
