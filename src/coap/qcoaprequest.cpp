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

#include "qcoaprequest_p.h"
#include <QtCore/qmath.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate(const QUrl &url, QCoapMessage::MessageType _type, const QUrl &proxyUrl) :
    uri(url),
    proxyUri(proxyUrl)
{
    type = _type;
}

QCoapRequestPrivate::~QCoapRequestPrivate()
{
}

/*!
    \class QCoapRequest
    \brief The QCoapRequest class holds a CoAP request. This request
    can be sent with QCoapClient.

    \reentrant

    The QCoapRequest contains data needed to make CoAP frames that can be
    sent to the url it holds.

    \sa QCoapClient, QCoapReply, QCoapDiscoveryReply
*/

/*!
    Constructs a QCoapRequest object with the target \a url,
    the proxy url \a proxyUrl and the \a type of the message.
*/
QCoapRequest::QCoapRequest(const QUrl &url, MessageType type, const QUrl &proxyUrl) :
    QCoapMessage(*new QCoapRequestPrivate(url, type, proxyUrl))
{
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // to generate message ids and tokens
}

/*!
    Constructs a copy of the \a other QCoapRequest. Optionally allows to
    overwrite the QCoapRequest::Operation of the request with the \a op
    argument.
*/
QCoapRequest::QCoapRequest(const QCoapRequest &other, QCoapRequest::Operation op) :
    QCoapMessage(other)
{
    if (op != QCoapRequest::Empty)
        setOperation(op);
}

/*!
    Returns the target URI of the request.

    \sa setUrl()
*/
QUrl QCoapRequest::url() const
{
    Q_D(const QCoapRequest);
    return d->uri;
}

/*!
    Returns the proxy URI of the request.
    The request shall be sent directly if this is invalid.

    \sa setProxyUrl()
*/
QUrl QCoapRequest::proxyUrl() const
{
    Q_D(const QCoapRequest);
    return d->proxyUri;
}

/*!
    Returns the operation of the request.

    \sa setOperation()
*/
QCoapRequest::Operation QCoapRequest::operation() const
{
    Q_D(const QCoapRequest);
    return d->operation;
}

/*!
    Returns true if the request is an observe request.

    \sa enableObserve()
*/
bool QCoapRequest::observe() const
{
    Q_D(const QCoapRequest);
    return d->observe;
}

/*!
    Sets the target URI of the request to the given \a url.

    \sa url()
*/
void QCoapRequest::setUrl(const QUrl &url)
{
    Q_D(QCoapRequest);
    d->uri = url;
}

/*!
    Sets the proxy URI of the request to the given \a proxyUrl.

    \sa proxyUrl()
*/
void QCoapRequest::setProxyUrl(const QUrl &proxyUrl)
{
    Q_D(QCoapRequest);
    d->proxyUri = proxyUrl;
}

/*!
    Sets the operation of the request to the given \a operation.

    \sa operation()
*/
void QCoapRequest::setOperation(Operation operation)
{
    Q_D(QCoapRequest);
    d->operation = operation;
}

/*!
    Sets the observe to true to make an observe request.

    \sa observe()
*/
void QCoapRequest::enableObserve()
{
    Q_D(QCoapRequest);
    d->observe = true;

    addOption(QCoapOption::Observe);
}

/*!
    Creates a copy of \a other.
*/
QCoapRequest &QCoapRequest::operator=(const QCoapRequest &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

/*!
    Returns true if this QCoapRequest has a lower message id than \a other has.
*/
bool QCoapRequest::operator<(const QCoapRequest &other) const
{
    return d_ptr->messageId < other.messageId();
}

/*!
    \internal

    For QSharedDataPointer.
*/
QCoapRequestPrivate* QCoapRequest::d_func()
{
    return static_cast<QCoapRequestPrivate*>(d_ptr.data());
}

QT_END_NAMESPACE
