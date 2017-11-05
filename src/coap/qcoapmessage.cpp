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

#include "qcoapmessage_p.h"

QT_BEGIN_NAMESPACE

#if 0
QCoapMessagePrivate::QCoapMessagePrivate(const QCoapMessagePrivate &other) :
    QSharedData(other),
    version(other.version),
    type(other.type),
    messageId(other.messageId),
    token(other.token),
    options(other.options),
    payload(other.payload)
{
}
#endif

/*!
    \class QCoapMessage
    \brief The QCoapMessage class holds information about a coap message that
    can be a request or a reply.

    \reentrant

    It holds information such as the message type, message id, token and other
    ancillary data.

    \sa QCoapOption, QCoapReply, QCoapRequest
*/

/*!
    \enum QCoapMessage::QCoapMessageType

    Indicates the type of the message.

    \value ConfirmableCoapMessage       A Confirmable message. The destination
                                        endpoint needs to acknowledge the
                                        message.
    \value NonConfirmableCoapMessage    A Non-Confirmable message. The
                                        destination endpoint does not need to
                                        acknowledge the message.
    \value AcknowledgementCoapMessage   An Acknowledgement message. A message
                                        sent or received in reply to a
                                        Confirmable message.
    \value ResetCoapMessage             A Reset message. A message sent
                                        sometimes in case of errors or to stop
                                        transmissions. (For example, it is used
                                        to cancel an observation).
*/

/*!
    Constructs a new QCoapMessage.
*/
QCoapMessage::QCoapMessage() :
    d_ptr(new QCoapMessagePrivate)
{
}

/*!
    Destroys the QCoapMessage.
*/
QCoapMessage::~QCoapMessage()
{
}

/*!
    Constructs a shallow copy of \a other.
*/
QCoapMessage::QCoapMessage(const QCoapMessage &other) :
    d_ptr(other.d_ptr)
{
}

/*!
    \internal
    Constructs a new QCoapMessage with \a dd as the d_ptr.
    This constructor must be used internally when subclassing
    the QCoapMessage class.
*/
QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd) :
    d_ptr(&dd)
{
}

/*!
    \overload

    Adds the coap option with the given \a name and \a value.
*/
void QCoapMessage::addOption(QCoapOption::OptionName name, const QByteArray &value)
{
    QCoapOption option(name, value);
    addOption(option);
}

/*!
    Adds the given coap \a option.
*/
void QCoapMessage::addOption(const QCoapOption &option)
{
    d_ptr->options.push_back(option);
}

/*!
    Removes the given \a option.
*/
void QCoapMessage::removeOption(const QCoapOption &option)
{
    d_ptr->options.removeOne(option);
}

/*!
    Removes the first option with the given \a name.
*/
void QCoapMessage::removeOption(QCoapOption::OptionName name)
{
    for (QCoapOption option : d_ptr->options) {
        if (option.name() == name) {
            removeOption(option);
            break;
        }
    }
}

/*!
    Finds and returns the option with the given \a name.
    If there is no such option, returns an Invalid CoapOption with an empty value.
*/
QCoapOption QCoapMessage::findOptionByName(QCoapOption::OptionName name)
{
    for (QCoapOption option : d_ptr->options) {
        if (option.name() == name)
            return option;
    }

    return QCoapOption();
}

/*!
    Removes all options.
*/
void QCoapMessage::removeAllOptions()
{
    d_ptr->options.clear();
}

/*!
    Returns the coap version.

    \sa setVersion()
*/
quint8 QCoapMessage::version() const
{
    return d_ptr->version;
}

/*!
    Returns the message type.

    \sa setType()
*/
QCoapMessage::MessageType QCoapMessage::type() const
{
    return d_ptr->type;
}

/*!
    Returns the message token.

    \sa setToken()
*/
QByteArray QCoapMessage::token() const
{
    return d_ptr->token;
}

/*!
    Returns the token length.
*/
quint8 QCoapMessage::tokenLength() const
{
    return static_cast<quint8>(d_ptr->token.length());
}

/*!
    Returns the message id.

    \sa setMessageId()
*/
quint16 QCoapMessage::messageId() const
{
    return d_ptr->messageId;
}

/*!
    Returns the payload.

    \sa setPayload()
*/
QByteArray QCoapMessage::payload() const
{
    return d_ptr->payload;
}

/*!
    Returns the option at \a index position.
*/
QCoapOption QCoapMessage::option(int index) const
{
    return d_ptr->options.at(index);
}

/*!
    Returns the list of options.
*/
QList<QCoapOption> QCoapMessage::optionList() const
{
    return d_ptr->options;
}

/*!
    Returns the number of options.
*/
int QCoapMessage::optionsLength() const
{
    return d_ptr->options.length();
}

/*!
    Sets the coap version.

    \sa version()
*/
void QCoapMessage::setVersion(quint8 version)
{
    d_ptr->version = version;
}

/*!
    Sets the message type.

    \sa type()
*/
void QCoapMessage::setType(const MessageType &type)
{
    d_ptr->type = type;
}

/*!
    Sets the token.

    \sa token()
*/
void QCoapMessage::setToken(const QByteArray &token)
{
    d_ptr->token = token;
}

/*!
    Sets the message id.

    \sa messageId()
*/
void QCoapMessage::setMessageId(quint16 id)
{
    d_ptr->messageId = id;
}

/*!
    Sets the payload.

    \sa payload()
*/
void QCoapMessage::setPayload(const QByteArray &payload)
{
    d_ptr->payload = payload;
}

void QCoapMessage::swap(QCoapMessage &other) Q_DECL_NOTHROW
{
    qSwap(d_ptr, other.d_ptr);
}

QCoapMessage &QCoapMessage::operator=(QCoapMessage &&other) Q_DECL_NOTHROW
{
    swap(other);
    return *this;
}

QCoapMessage &QCoapMessage::operator=(const QCoapMessage &other)
{
    d_ptr = other.d_ptr;
    return *this;
}
QT_END_NAMESPACE
