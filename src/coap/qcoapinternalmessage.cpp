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

#include "qcoapinternalmessage_p.h"

QT_BEGIN_NAMESPACE

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate() :
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate(const QCoapInternalMessagePrivate &other) :
    QObjectPrivate (other),
    message(other.message),
    currentBlockNumber(other.currentBlockNumber),
    hasNextBlock(other.hasNextBlock),
    blockSize(other.blockSize)
{
}

QCoapInternalMessagePrivate::~QCoapInternalMessagePrivate()
{
}

/*!
    \internal

    \class QCoapInternalMessage
    \brief The QCoapInternalMessage class contains data related to
    a received message or a message to send. It contains an instance of
    QCoapMessage and other data for the block management.

    \reentrant

    The QCoapInternalMessage class is inherited by QCoapInternalRequest and
    QCoapInternalReply that are used internally to manage requests to send
    and receive replies.

    \sa QCoapInternalReply, QCoapInternalRequest, QCoapMessage
*/

/*!
    \internal

    Constructs a new QCoapInternalMessage and sets \a parent as the parent
    object.
 */
QCoapInternalMessage::QCoapInternalMessage(QObject *parent) :
    QObject(* new QCoapInternalMessagePrivate, parent)
{
}

/*!
    \internal

    Constructs a new QCoapInternalMessage with the given \a message and sets
    \a parent as the parent object.
 */
QCoapInternalMessage::QCoapInternalMessage(const QCoapMessage &message, QObject *parent) :
    QCoapInternalMessage(parent)
{
    Q_D(QCoapInternalMessage);
    d->message = message;
}

/*!
    \internal

    Constructs a copy of \a other and sets \a parent as the parent object.
*/
QCoapInternalMessage::QCoapInternalMessage(const QCoapInternalMessage &other, QObject *parent) :
    QObject(* new QCoapInternalMessagePrivate(*other.d_func()), parent)
{
}

/*!
    \internal
    Constructs a new QCoapInternalMessage with \a dd as the d_ptr.
    This constructor must be used when subclassing internally
    the QCoapInternalMessage class.
*/
QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd, QObject *parent):
    QObject(dd, parent)
{
}

/*!
    \internal
    \overload

    Adds the coap option with the given \a name and \a value.
*/
void QCoapInternalMessage::addOption(QCoapOption::OptionName name, const QByteArray &value)
{
    QCoapOption option(name, value);
    addOption(option);
}

/*!
    \internal

    Adds the given coap \a option.
*/
void QCoapInternalMessage::addOption(const QCoapOption &option)
{
    Q_D(QCoapInternalMessage);
    d->message.addOption(option);
}

/*!
    \internal

    Removes the option with the given \a name.
*/
void QCoapInternalMessage::removeOption(QCoapOption::OptionName name)
{
    Q_D(QCoapInternalMessage);
    d->message.removeOption(name);
}

/*!
    \internal

    Returns a pointer to the message.
*/
QCoapMessage *QCoapInternalMessage::message()
{
    Q_D(QCoapInternalMessage);
    return &(d->message);
}

/*!
    \internal

    Returns a const pointer to the message.
*/
const QCoapMessage *QCoapInternalMessage::message() const
{
    Q_D(const QCoapInternalMessage);
    return &(d->message);
}

/*!
    \internal

    Returns the block number
*/
uint QCoapInternalMessage::currentBlockNumber() const
{
    Q_D(const QCoapInternalMessage);
    return d->currentBlockNumber;
}

/*!
    \internal

    Returns \c true if it has a next block,\c false otherwise.
*/
bool QCoapInternalMessage::hasNextBlock() const
{
    Q_D(const QCoapInternalMessage);
    return d->hasNextBlock;
}

/*!
    \internal

    Returns the size of the block.
*/
uint QCoapInternalMessage::blockSize() const
{
    Q_D(const QCoapInternalMessage);
    return d->blockSize;
}

QT_END_NAMESPACE
