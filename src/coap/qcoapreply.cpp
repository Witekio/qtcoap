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

#include "qcoapreply_p.h"
#include "qcoapinternalreply_p.h"
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \class QCoapReplyPrivate
    \brief Class's constructor
*/
QCoapReplyPrivate::QCoapReplyPrivate(const QCoapRequest &req) :
    request(req)
{
}

/*!
    \class QCoapReply
    \brief The QCoapReply class holds the data of a CoAP reply.

    \reentrant

    The QCoapReply contains data related to a request sent with the
    QCoapClient.

    The \l{QCoapReply::finished()}{finished()} signal is emitted when
    the response is fully received and when request fails.

    The \l{QCoapReply::notified(const QByteArray&)}
    {notified(const QByteArray&)} signal is emitted when a resource sends
    a notification in response of an observe request.

    \sa QCoapClient, QCoapRequest, QCoapDiscoveryReply
*/

//! TODO Document all enum values
/*!
    \enum QtCoap::Error

    Indicates all possible error conditions found during the
    processing of the request.

    \value NoError                  No error condition.

    \value HostNotFoundError        The remote host name was not
                                    found.

    \value BadRequestError          The request was not recognized.

    \value AddressInUseError        The address is already in use.

    \value TimeOutError             The response did not arrive in time.

    \value UnknownError             An unknown error was detected.

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

    This signal is emitted whenever the reply is finished. When a resource is
    observed, only the \l{QCoapReply::notified()}{notified()} will be emitted.

    \note Aborted replies send the \l{QCoapReply::aborted()}{aborted()} signal
    only.

    \sa QCoapClient::finished(), isFinished(), notified(), aborted()
*/

/*!
    \fn void QCoapReply::notified(const QByteArray &payload)

    This signal is emitted whenever a notification is received from a resource
    for an observe request.

    Its \a payload parameter is a QByteArray containing the payload of the
    notification.

    \sa QCoapClient::finished(), isFinished(), finished(), notified()
*/

/*!
    \fn void QCoapReply::error(QCoapReply::QCoapNetworkError error)

    This signal is emitted whenever an error is sent by the socket or the code
    received into the reply is an error code.

    Its \a error parameter is the error received.

    \sa finished(), aborted()
*/

/*!
    \fn void QCoapReply::aborted(const QCoapToken &token);

    This signal is emitted when the request is aborted or the reply is deleted.
    Given the QCoapReply may have been deleted at that point, you should not
    rely on the sender() object to be still valid.

    Its \a token parameter is the token of the exchange that have been aborted.

    \sa finished(), error()
*/

/*!
    Constructs a QCoapReply object and sets \a parent as the parent object.
*/
QCoapReply::QCoapReply(const QCoapRequest &request, QObject *parent) :
    QCoapReply(* new QCoapReplyPrivate(request), parent)
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
    Destroys the QCoapReply object and aborts the request if its response has
    not yet been received.
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

    // Ensure memcpy is compatible with a qint64 length.
    // FIXME Isn't it going to be a problem on ARM based platforms?
    Q_STATIC_ASSERT(sizeof(size_t) >= sizeof(qint64));
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
    return d->isFinished || d->isAborted;
}

/*!
    Returns true if the request is running.
*/
bool QCoapReply::isRunning() const
{
    Q_D(const QCoapReply);
    return d->isRunning && !isFinished();
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
    Returns the method of the associated request.
*/
QtCoap::Method QCoapReply::method() const
{
    Q_D(const QCoapReply);
    return d->request.method();
}

/*!
    Returns the error of the reply or QCoapReply::NoError if there is no error.
*/
QtCoap::Error QCoapReply::errorReceived() const
{
    Q_D(const QCoapReply);
    return d->error;
}

/*!
    Sets the isRunning parameter to the given \a isRunning value.

    \sa isRunning()
*/
void QCoapReply::setRunning(const QCoapToken &token, QCoapMessageId messageId)
{
    Q_D(QCoapReply);
    d->request.setToken(token);
    d->request.setMessageId(messageId);
    d->isRunning = true;
}

/*!
    \internal

    Sets the error of the reply.

    \sa errorReceived()
*/
void QCoapReply::setError(QtCoap::Error newError)
{
    Q_D(QCoapReply);
    if (d->error == newError)
        return;

    d->error = newError;
    emit error(this, d->error);
}

/*!
    \internal

    Sets the error of the reply.
*/
void QCoapReply::setError(QtCoap::StatusCode statusCode)
{
    setError(QtCoap::statusCodeError(statusCode));
}

/*!
    \internal

    Sets the reply as finished.
*/
void QCoapReply::setObserveCancelled()
{
    Q_D(QCoapReply);

    bool alreadyFinished = isFinished();

    d->isFinished = true;
    d->isRunning = false;

    if (!alreadyFinished)
        emit finished(this);
}

/*!
    \internal

    Updates the QCoapReply object and its message with data of the internal
    reply \a internalReply, unless this QCoapReply object has been aborted.
*/
void QCoapReply::setContent(const QCoapInternalReply *internalReply)
{
    Q_D(QCoapReply);

    if (!internalReply)
        return;

    d->message = *internalReply->message();
    d->status = internalReply->statusCode();

    if (QtCoap::isError(d->status))
        setError(d->status);
}

/*!
    \internal

    Sets the reply as finished, sending the finished() signal.
*/
void QCoapReply::setFinished(QtCoap::Error newError)
{
    Q_D(QCoapReply);

    if (isFinished())
        return;

    d->isFinished = true;
    d->isRunning = false;

    if (newError != QtCoap::NoError)
        setError(newError);

    emit finished(this);
}

/*!
    \internal

    Updates the QCoapReply object and its message with data of the internal
    reply \a internalReply, unless this QCoapReply object has been aborted.
*/
void QCoapReply::setNotified(const QCoapInternalReply *internalReply)
{
    Q_D(QCoapReply);

    if (!internalReply || isFinished())
        return;

    d->message = *internalReply->message();
    d->status = internalReply->statusCode();

    emit notified(this, d->message);
}

/*!
    Aborts the request immediately and emits the
    \l{QCoapReply::aborted(const QCoapToken &token)}{aborted(const QCoapToken &token)}
    signal if the request was not finished.
*/
void QCoapReply::abortRequest()
{
    Q_D(QCoapReply);

    if (isFinished())
        return;

    d->isAborted = true;
    d->isFinished = true;
    d->isRunning = false;
    emit aborted(request().token());
    emit finished(this);
}

QT_END_NAMESPACE
