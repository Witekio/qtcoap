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
#include "qcoapinternalrequest_p.h"
#include <QtCore/qmath.h>
#include <QtCore/qdatetime.h>

QT_BEGIN_NAMESPACE

QCoapRequestPrivate::QCoapRequestPrivate() :
    uri(QUrl()),
    proxyUri(QUrl()),
    operation(QCoapRequest::Empty),
    observe(false)
{
}

QCoapRequestPrivate::QCoapRequestPrivate(const QCoapRequestPrivate &other) :
    QCoapMessagePrivate(other),
    uri(other.uri),
    proxyUri(other.proxyUri),
    operation(other.operation),
    observe(other.observe)
{
}

/*!
    \class QCoapRequest
    \brief The QCoapRequest class holds a coap request. This request
    can be sent with QCoapClient.

    \reentrant

    The QCoapRequest contains data needed to make coap frames that can be
    sent to the url it holds.

    \sa QCoapClient, QCoapReply, QCoapDiscoveryReply
*/

/*!
    Constructs a QCoapRequest object with the target \a url,
    the proxy url \a proxyUrl and the \a type of the message.
*/
QCoapRequest::QCoapRequest(const QUrl &url, MessageType type, const QUrl &proxyUrl) :
    QCoapMessage(*new QCoapRequestPrivate)
{
    setUrl(url);
    setProxyUrl(proxyUrl);
    setType(type);
    qsrand(static_cast<uint>(QTime::currentTime().msec())); // to generate message ids and tokens
}

/*!
    Constructs a copy of the \a other QCoapRequest
*/
QCoapRequest::QCoapRequest(const QCoapRequest &other) :
    QCoapMessage (other)
{
}

/*!
    Returns the target uri of the request.

    \sa setUrl()
*/
QUrl QCoapRequest::url() const
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->uri;
}

/*!
    Returns the proxy uri of the request.

    \sa setProxyUrl()
*/
QUrl QCoapRequest::proxyUrl() const
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->proxyUri;
}

/*!
    Returns the operation of the request.

    \sa setOperation()
*/
QCoapRequest::Operation QCoapRequest::operation() const
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->operation;
}

/*!
    Returns true if the request is an observe request.

    \sa setObserve()
*/
bool QCoapRequest::observe() const
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    return d->observe;
}

/*!
    Sets the target uri of the request to the given \a url.

    \sa url()
*/
void QCoapRequest::setUrl(const QUrl &url)
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    d->uri = url;
}

/*!
    Sets the proxy uri of the request to the given \a proxyUrl.

    \sa proxyUrl()
*/
void QCoapRequest::setProxyUrl(const QUrl &proxyUrl)
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    d->proxyUri = proxyUrl;
}

/*!
    Sets the operation of the request to the given \a operation.

    \sa operation()
*/
void QCoapRequest::setOperation(Operation operation)
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    d->operation = operation;
}

/*!
    Sets the observe parameter of the request to the given \a observe value.
    Sets the observe to true to make an observe request.

    \sa observe()
*/
void QCoapRequest::setObserve(bool observe)
{
    QCoapRequestPrivate *d = static_cast<QCoapRequestPrivate*>(d_ptr);
    d->observe = observe;
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
    Returns true if this QCoapRequest message id is lower than
    the message id of \a other.
*/
bool QCoapRequest::operator<(const QCoapRequest &other) const
{
    return (d_ptr->messageId < other.messageId());
}

QT_END_NAMESPACE
