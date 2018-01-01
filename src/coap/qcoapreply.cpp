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
    \internal
    Sets the isRunning parameter to the given \a isRunning value.

    \sa isRunning()
*/
void QCoapReplyPrivate::_q_setRunning(const QCoapToken &token, QCoapMessageId messageId)
{
    request.setToken(token);
    request.setMessageId(messageId);
    isRunning = true;
}

/*!
    \internal

    Sets the reply as finished.
*/
void QCoapReplyPrivate::_q_setObserveCancelled()
{
    Q_Q(QCoapReply);

    bool alreadyFinished = q->isFinished();

    isFinished = true;
    isRunning = false;

    if (!alreadyFinished)
        emit q->finished(q);
}

/*!
    \internal

    Updates the QCoapReply object and its message with data of the internal
    reply \a internalReply, unless this QCoapReply object has been aborted.
*/
void QCoapReplyPrivate::_q_setContent(const QCoapMessage &msg, QtCoap::StatusCode status)
{
    message = msg;
    statusCode = status;
    seekBuffer(0);

    if (QtCoap::isError(statusCode))
        _q_setError(statusCode);
}

/*!
    \internal

    Updates the QCoapReply object and its message with data of the internal
    reply \a internalReply, unless this QCoapReply object has been aborted.
*/
void QCoapReplyPrivate::_q_setNotified()
{
    Q_Q(QCoapReply);

    if (!q->isFinished())
        emit q->notified(q, message);
}

/*!
    \internal

    Sets the reply as finished, sending the finished() signal.
*/
void QCoapReplyPrivate::_q_setFinished(QtCoap::Error newError)
{
    Q_Q(QCoapReply);

    if (q->isFinished())
        return;

    isFinished = true;
    isRunning = false;

    if (newError != QtCoap::NoError)
        _q_setError(newError);

    emit q->finished(q);
}

/*!
    \internal

    Sets the error of the reply.

    \sa errorReceived()
*/
void QCoapReplyPrivate::_q_setError(QtCoap::Error newError)
{
    Q_Q(QCoapReply);
    if (error == newError)
        return;

    error = newError;
    emit q->error(q, error);
}

/*!
    \internal

    Sets the error of the reply.
*/
void QCoapReplyPrivate::_q_setError(QtCoap::StatusCode statusCode)
{
    _q_setError(QtCoap::statusCodeError(statusCode));
}

/*!
    \class QCoapReply
    \brief The QCoapReply class holds the data of a CoAP reply.

    \reentrant

    The QCoapReply contains data related to a request sent with the
    QCoapClient.

    The \l{QCoapReply::finished(QCoapReply*)}{finished(QCoapReply*)} signal is
    emitted when the response is fully received and when request fails.

    For Observe requests specifically, the
    \l{QCoapReply::notified(QCoapReply*, const QByteArray&)}{notified(QCoapReply*, const QByteArray&)}
    signal is emitted whenever a notification is received.

    \sa QCoapClient, QCoapRequest, QCoapDiscoveryReply
*/

/*!
    \fn void QCoapReply::finished(QCoapReply* reply)

    This signal is emitted whenever the corresponding request finished,
    either successfully or not. When a resource is observed, this signal will
    be emitted only once, in the same conditions.

    The \a reply parameter is the QCoapReply itself for convenience.

    \sa QCoapClient::finished(), isFinished(), notified(), aborted()
*/

/*!
    \fn void QCoapReply::notified(QCoapReply* reply, const QCoapMessage &message)

    This signal is emitted whenever a notification is received from an observed
    resource.

    Its \a message parameter is a QCoapMessage containing the payload and the
    message details. The \a reply parameter is the QCoapReply itself for
    convenience.

    \sa QCoapClient::finished(), isFinished(), finished(), notified()
*/

/*!
    \fn void QCoapReply::error(QCoapReply* reply, QtCoap::Error error)

    This signal is emitted whenever an error occurs and is followed by the
    finished() signal.

    Its \a reply parameters is the QCoapReply itself for convenience, and
    the \a error parameter is the error received.

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
    return d->statusCode;
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

#include "moc_qcoapreply.cpp"
