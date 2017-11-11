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

#include "qcoapreply_p.h"
#include "qcoapinternalreply_p.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCoapReply
    \brief The QCoapReply class holds the data of a CoAP reply.

    \reentrant

    The QCoapReply contains data related to a request sent with the
    QCoapClient.

    The \l{QCoapReply::finished()}{finished()} signal is emitted when
    the response is fully received and when request fails.

    The \l{QCoapReply::notified(const QByteArray&)}
    {notified(const QByteArray&)} signal is emitted when a resource send
    a notification in response of an observe request.

    \sa QCoapClient, QCoapRequest, QCoapDiscoveryReply
*/

/*!
    \enum QNetworkReply::NetworkError

    Indicates all possible error conditions found during the
    processing of the request.

    \value NoCoapError              no error condition.

    \value HostNotFoundCoapError    the remote host name was not
                                    found.

    \value BadRequestCoapError      the request was not recognized.

    \value AddressInUseCoapError    the address is already in use.

    \value TimeOutCoapError         the response did not arrive in time.

    \value UnknownCoapError         an unknown error was detected.

    \sa error()
*/
/*!
    \enum QtCoap::StatusCode

    This enum maps the status code of the CoAP protocol, as defined in
    the 'response' section of the
    \l{https://tools.ietf.org/html/rfc7252#section-5.2}{RFC 7252}
*/

/*!
    \fn void QCoapReply::finished()

    This signal is emitted whenever the reply is finished.

    \sa QCoapClient::finished(), isFinished(), notified()
*/

/*!
    \fn void QCoapReply::notified(const QByteArray&)

    This signal is emitted whenever a notification is received from a resource
    for an observe request.

    Its parameter is a byte array containing the payload of the notification.

    \sa QCoapClient::finished(), isFinished(), finished()
*/

/*!
    \fn void QCoapReply::error(QCoapReply::QCoapNetworkError)

    This signal is emitted whenever an error is sent by the socket or the code
    received into the reply is an error code.

    Its parameter is the error received.
*/

/*!
    \fn void QCoapReply::aborted(QCoapReply*);

    This signal is emitted when the request is aborted or the reply is deleted.

    Its parameter is the reply object related to the aborted request.
*/

/*!
    Constructs a QCoapReply object and sets \a parent as the parent object.
*/
QCoapReply::QCoapReply(QObject *parent) :
    QCoapReply(* new QCoapReplyPrivate, parent)
{
}

/*!
    \internal
    Constructs a new QCoapReply with \a dd as the d_ptr.
    This constructor must be used when subclassing internally
    the QCoapReply class.
*/
QCoapReply::QCoapReply(QCoapReplyPrivate &dd, QObject *parent) :
    QIODevice(dd, parent)
{
    open(QIODevice::ReadOnly);
}

/*!
    Destroys the QCoapReply object and abort the request in case of the
    response has not been received yet.
*/
QCoapReply::~QCoapReply()
{
    abortRequest();
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::readData(char *data, qint64 maxSize)
{
    Q_D(QCoapReply);

    QByteArray payload = d->message.payload();

    qint64 len = qMin(maxSize, qint64(payload.size()) - pos());
    if (len <= 0)
        return qint64(0);
    memcpy(data, payload.constData() + pos(), static_cast<size_t>(len));

    return len;
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::writeData(const char *data, qint64 maxSize)
{
    // The user cannot write to the reply
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

/*!
    Returns the status code of the request.
*/
QtCoap::StatusCode QCoapReply::statusCode() const
{
    Q_D(const QCoapReply);
    return d->status;
}

/*!
    Returns the contained message.
*/
QCoapMessage QCoapReply::message() const
{
    Q_D(const QCoapReply);
    return d->message;
}

/*!
    Returns the associated request.

    \sa setRequest()
*/
QCoapRequest QCoapReply::request() const
{
    Q_D(const QCoapReply);
    return d->request;
}

/*!
    Returns true if the request is finished.

    \sa finished()
*/
bool QCoapReply::isFinished() const
{
    Q_D(const QCoapReply);
    return d->isFinished;
}

/*!
    Returns true if the request is running.
*/
bool QCoapReply::isRunning() const
{
    Q_D(const QCoapReply);
    return d->isRunning;
}

/*!
    Returns true if the request has been aborted.
*/
bool QCoapReply::isAborted() const
{
    Q_D(const QCoapReply);
    return d->isAborted;
}

/*!
    Returns the target uri of the associated request.
*/
QUrl QCoapReply::url() const
{
    Q_D(const QCoapReply);
    return d->request.url();
}

/*!
    Returns the operation of the associated request.
*/
QCoapRequest::Operation QCoapReply::operation() const
{
    Q_D(const QCoapReply);
    return d->request.operation();
}

/*!
    Returns the error of the reply or NoCoapError if there is no error.
*/
QCoapReply::NetworkError QCoapReply::errorReceived() const
{
    Q_D(const QCoapReply);
    return d->error;
}

/*!
    Sets the request associated to the this QCoapReply to the given \a request.

    \sa request()
*/
void QCoapReply::setRequest(const QCoapRequest &request)
{
    Q_D(QCoapReply);
    d->request = request;
}

/*!
    Sets the isRunning parameter to the given \a isRunning value.

    \sa isRunning()
*/
void QCoapReply::setIsRunning(bool isRunning)
{
    Q_D(QCoapReply);
    d->isRunning = isRunning;
}

/*!
    Sets the error parameter to \a newError.

    \sa errorReceived()
*/
void QCoapReply::setError(NetworkError newError)
{
    Q_D(QCoapReply);
    if (d->error == newError)
        return;

    d->error = newError;
    emit error(d->error);
}

/*!
    \internal

    Updates the QCoapReply object and its message with data of the internal
    reply \a internalReply.
*/
void QCoapReply::updateFromInternalReply(const QCoapInternalReply &internalReply)
{
    Q_D(QCoapReply);

    if (!d->isAborted) {
        const QCoapMessage *internalReplyMessage = internalReply.message();

        d->message.setPayload(internalReplyMessage->payload());
        d->message.setType(internalReplyMessage->type());
        d->message.setVersion(internalReplyMessage->version());
        d->status = internalReply.statusCode();
        d->isFinished = true;
        d->isRunning = false;

        if (d->status >= QtCoap::BadRequest)
            replyError(d->status);

        if (d->request.observe())
            emit notified(internalReplyMessage->payload());

        emit finished();
    }
}

/*!
    Aborts the operation immediately and emits the
    \l{QCoapReply::aborted(QCoapReply*)}{aborted(QCoapReply*)} signal
    if the request was not finished before.
*/
void QCoapReply::abortRequest()
{
    Q_D(QCoapReply);
    d->isAborted = true;
    if (!this->isFinished())
        emit aborted(this);
}

/*!
    \internal

    Maps the reply status code \a errorCode to the related CoAP network error.
*/
void QCoapReply::replyError(QtCoap::StatusCode errorCode)
{
    NetworkError networkError;
    switch (errorCode) {
    case QtCoap::BadRequest:
        networkError = BadRequestError;
        break;
    case QtCoap::Unauthorized:
        networkError = UnauthorizedError;
        break;
    case QtCoap::BadOption:
        networkError = BadOptionError;
        break;
    case QtCoap::Forbidden:
        networkError = ForbiddenError;
        break;
    case QtCoap::NotFound:
        networkError = NotFoundError;
        break;
    case QtCoap::MethodNotAllowed:
        networkError = MethodNotAllowedError;
        break;
    case QtCoap::NotAcceptable:
        networkError = NotAcceptableError;
        break;
    case QtCoap::RequestEntityIncomplete:
        networkError = RequestEntityIncompleteError;
        break;
    case QtCoap::PreconditionFailed:
        networkError = PreconditionFailedError;
        break;
    case QtCoap::RequestEntityTooLarge:
        networkError = RequestEntityTooLargeError;
        break;
    case QtCoap::UnsupportedContentFormat:
        networkError = UnsupportedContentFormatError;
        break;
    case QtCoap::InternalServerError:
        networkError = InternalServerErrorError;
        break;
    case QtCoap::NotImplemented:
        networkError = NotImplementedError;
        break;
    case QtCoap::BadGateway:
        networkError = BadGatewayError;
        break;
    case QtCoap::ServiceUnavailable:
        networkError = ServiceUnavailableError;
        break;
    case QtCoap::GatewayTimeout:
        networkError = GatewayTimeoutError;
        break;
    case QtCoap::ProxyingNotSupported:
        networkError = ProxyingNotSupportedError;
        break;
    default:
        networkError = UnknownError;
    }

    setError(networkError);
}

/*!
    \internal

    Maps the socket error \a socketError to the related CoAP network error.
*/
void QCoapReply::connectionError(QAbstractSocket::SocketError socketError)
{
    NetworkError networkError;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError :
        networkError = HostNotFoundError;
        break;
    case QAbstractSocket::AddressInUseError :
        networkError = AddressInUseError;
        break;
    default:
        networkError = UnknownError;
    }

    setError(networkError);
}

QT_END_NAMESPACE
