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

#include "qcoapinternalreply_p.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

#include "qcoapparser_p.h"

/*!
    \internal

    Constructs a new QCoapInternalReplyPrivate from an existing \a message.
*/
QCoapInternalReplyPrivate::QCoapInternalReplyPrivate(const QCoapMessage &message) :
    QCoapInternalMessagePrivate(message)
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
    QCoapInternalMessage(*new QCoapInternalReplyPrivate, parent)
{
}

/*!
    \internal
    Constructs a copy of \a other with an optional \a parent.
*/
QCoapInternalReply::QCoapInternalReply(const QCoapInternalReply &other, QObject *parent) :
    QCoapInternalMessage(*new QCoapInternalReplyPrivate(*other.d_func()), parent)
{
}

/*!
    \internal
    Constructs a new QCoapInternalReply with a \a message and an optional \a parent.
*/
QCoapInternalReply::QCoapInternalReply(const QCoapMessage &msg, QObject *parent) :
    QCoapInternalMessage(*new QCoapInternalReplyPrivate(msg), parent)
{
    updateFromDescriptiveBlockOption(message()->option(QCoapOption::Block2));
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
*/
void QCoapInternalReply::addOption(const QCoapOption &option)
{
    QCoapInternalMessage::addOption(option);

    if (option.name() == QCoapOption::Block2)
        updateFromDescriptiveBlockOption(QCoapOption::Block2);
}

/*!
    \internal
    Sets the response code.
*/
void QCoapInternalReply::setResponseCode(QtCoap::ResponseCode code)
{
    Q_D(QCoapInternalReply);
    d->responseCode = code;
}

/*!
    \internal
    Sets the sender address.
*/
void QCoapInternalReply::setSenderAddress(const QHostAddress &address)
{
    Q_D(QCoapInternalReply);
    d->senderAddress = address;
}

/*!
    \internal
    Returns the number of the next block, if there is another block to come,
    otherwise -1.
    For more details, see QCoapParser::nextBlockToSend().
*/
int QCoapInternalReply::nextBlockToSend() const
{
    Q_D(const QCoapInternalReply);
    return d->parser->nextBlockToSend(d->message.option(QCoapOption::Block1));
}

/*!
    \internal
    Returns \c true if the client has one or more blocks to send.
*/
bool QCoapInternalReply::hasMoreBlocksToSend() const
{
    return nextBlockToSend() >= 0;
}

/*!
    \internal
    Returns the response code of the reply.
*/
QtCoap::ResponseCode QCoapInternalReply::responseCode() const
{
    Q_D(const QCoapInternalReply);
    return d->responseCode;
}

/*!
    \internal
    Returns the host address from which the reply was received.
*/
QHostAddress QCoapInternalReply::senderAddress() const
{
    Q_D(const QCoapInternalReply);
    return d->senderAddress;
}

QT_END_NAMESPACE
