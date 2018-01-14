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

#include "qcoapclient_p.h"
#include "qcoapreply.h"
#include "qcoapdiscoveryreply.h"
#include "qcoapnamespace.h"
#include <QtCore/qurl.h>
#include <QtNetwork/qudpsocket.h>

QT_BEGIN_NAMESPACE

QRandomGenerator QtCoap::randomGenerator;

QCoapClientPrivate::QCoapClientPrivate(QCoapProtocol *protocol, QCoapConnection *connection) :
    protocol(protocol),
    connection(connection),
    workerThread(new QThread)
{
    protocol->moveToThread(workerThread);
    connection->moveToThread(workerThread);
    workerThread->start();
}

QCoapClientPrivate::~QCoapClientPrivate()
{
    workerThread->quit();
    workerThread->wait();
    delete workerThread;
    delete protocol;
    delete connection;
}

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
    \class QtCoap

    Returns the QtCoap::Error corresponding to the \a code passed to this
    method.
*/
QtCoap::Error QtCoap::statusCodeError(QtCoap::StatusCode code)
{
    if (!isError(code))
        return QtCoap::NoError;

    switch (code) {
    case QtCoap::BadRequest:
        return BadRequestError;
    case QtCoap::Unauthorized:
        return UnauthorizedError;
    case QtCoap::BadOption:
        return BadOptionError;
    case QtCoap::Forbidden:
        return ForbiddenError;
    case QtCoap::NotFound:
        return NotFoundError;
    case QtCoap::MethodNotAllowed:
        return MethodNotAllowedError;
    case QtCoap::NotAcceptable:
        return NotAcceptableError;
    case QtCoap::RequestEntityIncomplete:
        return RequestEntityIncompleteError;
    case QtCoap::PreconditionFailed:
        return PreconditionFailedError;
    case QtCoap::RequestEntityTooLarge:
        return RequestEntityTooLargeError;
    case QtCoap::UnsupportedContentFormat:
        return UnsupportedContentFormatError;
    case QtCoap::InternalServerError:
        return InternalServerErrorError;
    case QtCoap::NotImplemented:
        return NotImplementedError;
    case QtCoap::BadGateway:
        return BadGatewayError;
    case QtCoap::ServiceUnavailable:
        return ServiceUnavailableError;
    case QtCoap::GatewayTimeout:
        return GatewayTimeoutError;
    case QtCoap::ProxyingNotSupported:
        return ProxyingNotSupportedError;
    default:
        return UnknownError;
    }
}

/*!
    \class QCoapClient
    \brief The QCoapClient class allows the application to
    send CoAP requests and receive replies.

    \reentrant

    The QCoapClient class contains signals that get triggered when the
    reply of a sent request has arrived.

    The application can use a QCoapClient to send requests over a CoAP
    network. It provides functions for standard requests: each returns a QCoapReply object,
    to which the response data shall be delivered; this can be read when the finished()
    signal arrives.

    A simple request can be sent with:
    \code
        QCoapClient *client = new QCoapClient(this);
        connect(client, &QCoapClient::finished, this, &TestClass::slotFinished);
        client->get(QCoapRequest(Qurl("coap://coap.me/test")));
    \endcode

    You can also use an "observe" request. This can be used as above, or more
    conveniently with the \l{QCoapReply::notified(const QByteArray&)}{notified(const QByteArray&)}
    signal:
    \code
        QCoapRequest request = QCoapRequest(Qurl("coap://coap.me/obs"));
        CoapReply *reply = client->observe(request);
        connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);
    \endcode

    And the observation can be cancelled with:
    \code
        client->cancelObserve(reply);
    \endcode

    When a reply arrives, the QCoapClient emits a finished(QCoapReply *) signal.

    \note For a discovery request, the returned object is a QCoapDiscoveryReply.
    It can be used the same way as a QCoapReply but contains also a list of
    resources.

    \sa QCoapRequest, QCoapReply, QCoapDiscoveryReply
*/

/*!
    \fn void QCoapClient::finished(QCoapReply *reply)

    This signal is emitted along with the \l{QCoapReply::finished()} signal
    whenever a CoAP reply is finished, after either a success or an error.
    The \a reply parameter will contain a pointer to the reply that has just
    finished.

    \sa error(), QCoapReply::finished(), QCoapReply::error()
*/

/*!
    \fn void QCoapClient::error(QCoapReply *reply, QtCoap::Error error)

    This signal is emitted whenever an error occurs. The \a reply parameter
    can be null if the error is not related to a specific QCoapReply.

    \sa finished(), QCoapReply::error(), QCoapReply::finished()
*/

/*!
    Constructs a QCoapClient object and sets \a parent as the parent object.
*/
QCoapClient::QCoapClient(QObject *parent) :
    QCoapClient(new QCoapProtocol, new QCoapConnection, parent)
{
}

/*!
    Base constructor, taking the \a protocol, \a connection, and \a parent
    as arguments.
*/
QCoapClient::QCoapClient(QCoapProtocol *protocol, QCoapConnection *connection, QObject *parent) :
    QObject(* new QCoapClientPrivate(protocol, connection), parent)
{
    Q_D(QCoapClient);

    qRegisterMetaType<QCoapReply*>();
    qRegisterMetaType<QCoapMessage>();
    qRegisterMetaType<QPointer<QCoapReply>>();
    qRegisterMetaType<QPointer<QCoapDiscoveryReply>>();
    qRegisterMetaType<QCoapConnection*>();
    qRegisterMetaType<QtCoap::Error>();
    qRegisterMetaType<QtCoap::StatusCode>();
    // Requires a name, as this is a typedef
    qRegisterMetaType<QCoapToken>("QCoapToken");
    qRegisterMetaType<QCoapMessageId>("QCoapMessageId");
    qRegisterMetaType<QAbstractSocket::SocketOption>();

    connect(d->connection, SIGNAL(readyRead(const QNetworkDatagram&)),
            d->protocol, SLOT(onFrameReceived(const QNetworkDatagram&)));
    connect(d->connection, SIGNAL(error(QAbstractSocket::SocketError)),
            d->protocol, SLOT(onConnectionError(QAbstractSocket::SocketError)));

    connect(d->protocol, &QCoapProtocol::finished,
            this, &QCoapClient::finished);
    connect(d->protocol, &QCoapProtocol::error,
            this, &QCoapClient::error);
}

/*!
    Destroys the QCoapClient object and frees up any
    resources. Note that QCoapReply objects that are returned from
    this class have the QCoapClient set as their parents, which means that
    they will be deleted along with it.
*/
QCoapClient::~QCoapClient()
{
    qDeleteAll(findChildren<QCoapReply*>(QString(), Qt::FindDirectChildrenOnly));
}

/*!
    Sends \a request using the GET method and returns a new QCoapReply object.

    \sa post(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::get(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    if (request.method() != QtCoap::Invalid
            && request.method() != QtCoap::Get) {
        qWarning("QCoapClient::get: Overriding method specified on request:"
                 "using 'Get' instead.");
    }

    QCoapRequest copyRequest(request, QtCoap::Get);

    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends request to \a url using the GET method and returns a new QCoapReply
    object.

    \sa post(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::get(const QUrl &url)
{
    QCoapRequest request(url);
    return get(request);
}

/*!
    Sends \a request using the PUT method and returns a new QCoapReply object.
    Uses \a data as the payload for this request.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    if (request.method() != QtCoap::Invalid
            && request.method() != QtCoap::Put) {
        qWarning("QCoapClient::put: Overriding method specified on request:"
                 "using 'Put' instead.");
    }

    QCoapRequest copyRequest(request, QtCoap::Put);
    copyRequest.setPayload(data);

    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends \a request using the PUT method and returns a new QCoapReply object.
    Uses \a device content as the payload for this request. A null device is
    treated as empty content.

    \note The device has to be open and readable before calling this function.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, QIODevice *device)
{
     return put(request, device ? device->readAll() : QByteArray());
}

/*!
    \overload

    Sends request to \a url using the PUT method and returns a new QCoapReply
    object. Uses \a data as the payload for this request.

    \sa get(), post(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::put(const QUrl &url, const QByteArray &data)
{
     return put(QCoapRequest(url), data);
}

/*!
    Posts a POST request sending the contents of the \a data QByteArray for the
    \a request, and returns a new QCoapReply object.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    if (request.method() != QtCoap::Invalid
            && request.method() != QtCoap::Post) {
        qWarning("QCoapClient::post: Overriding method specified on request:"
                 "using 'Post' instead.");
    }

    QCoapRequest copyRequest(request, QtCoap::Post);
    copyRequest.setPayload(data);

    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Posts a POST request sending the contents of the \a data device to the
    target \a request. If the device is null, then it returns a nullptr.

    \note The device has to be open and readable before calling this function.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, QIODevice *device)
{
    if (!device)
        return nullptr;

    return post(request, device->readAll());
}

/*!
    \overload

    Sends a POST request to the target \a url with the \a data QByteArray,
    and returns a new QCoapReply object.

    \sa get(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::post(const QUrl &url, const QByteArray &data)
{
    return post(QCoapRequest(url), data);
}

/*!
    Sends a DELETE request to the target of \a request.

    \sa get(), put(), post(), observe(), discover()
 */
QCoapReply *QCoapClient::deleteResource(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    if (request.method() != QtCoap::Invalid
            && request.method() != QtCoap::Delete) {
        qWarning("QCoapClient::deleteResource: Overriding method specified on request:"
                 "using 'Delete' instead.");
    }

    QCoapRequest copyRequest(request, QtCoap::Delete);

    return d->sendRequest(copyRequest);
}

/*!
    \overload

    Sends a DELETE request to the target \a url.

    \sa get(), put(), post(), observe(), discover()
 */
QCoapReply *QCoapClient::deleteResource(const QUrl &url)
{
    return deleteResource(QCoapRequest(url));
}

//! TODO discover should probably use a signal different from
//! 'finished', in order to be able to report multiple discovery
//! answers, from different CoAP servers.
/*!
    Discovers the resources available at the given \a url and returns
    a new QCoapDiscoveryReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response
    arrives.

    Discovery path defaults to "/.well-known/core", but can be changed
    by passing a different path to \a discoveryPath. Discovery is described in
    \l{https://tools.ietf.org/html/rfc6690#section-1.2.1}{RFC 6690}.

    \sa get(), post(), put(), deleteResource(), observe()
*/
QCoapDiscoveryReply *QCoapClient::discover(const QUrl &url, const QString &discoveryPath)
{
    Q_D(QCoapClient);

    QUrl discoveryUrl(url);
    discoveryUrl.setPath(url.path() + discoveryPath);

    QCoapRequest request(discoveryUrl);
    request.setMethod(QtCoap::Get);

    return d->sendDiscovery(request);
}

/*!
    Sends a request to observe the target \a request and returns
    a new QCoapReply object which emits the
    \l{QCoapReply::notified(const QByteArray&)}{notified(const QByteArray&)}
    signal whenever a new notification arrives.

    \sa cancelObserve(), get(), post(), put(), deleteResource(), discover()
*/
QCoapReply *QCoapClient::observe(const QCoapRequest &request)
{
    if (request.method() != QtCoap::Invalid
            && request.method() != QtCoap::Get) {
        qWarning("QCoapClient::observe: Overriding method specified on request:"
                 "using 'Get' instead.");
    }

    QCoapRequest copyRequest(request, QtCoap::Get);
    copyRequest.enableObserve();

    return get(copyRequest);
}

/*!
    \overload

    Sends a request to observe the target \a url and returns
    a new QCoapReply object which emits the
    \l{QCoapReply::notified(const QByteArray&)}{notified(const QByteArray&)}
    signal whenever a new notification arrives.

    \sa cancelObserve(), get(), post(), put(), deleteResource(), discover()
*/
QCoapReply *QCoapClient::observe(const QUrl &url)
{
    return observe(QCoapRequest(url));
}

/*!
    \overload

    Sends a request to cancel the observation of the target used by the
    reply \a notifiedReply

    \sa observe()
*/
void QCoapClient::cancelObserve(QCoapReply *notifiedReply)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "cancelObserve",
                              Q_ARG(QPointer<QCoapReply>, QPointer<QCoapReply>(notifiedReply)));
}

/*!
    \internal

    Sends the CoAP \a request to its own URL and returns a new QCoapReply
    object.
*/
QCoapReply *QCoapClientPrivate::sendRequest(QCoapRequest &request)
{
    Q_Q(QCoapClient);

    // Prepare the reply
    QCoapReply *reply = new QCoapReply(request, q);

    if (!send(reply)) {
        delete reply;
        return nullptr;
    }

    return reply;
}

/*!
    \internal

    Sends the CoAP \a request to its own URL and returns a
    new QCoapDiscoveryReply object.
*/
QCoapDiscoveryReply *QCoapClientPrivate::sendDiscovery(QCoapRequest &request)
{
    Q_Q(QCoapClient);

    // Prepare the reply
    QCoapDiscoveryReply *reply = new QCoapDiscoveryReply(request, q);

    if (!send(reply)) {
        delete reply;
        return nullptr;
    }

    return reply;
}

/*!
    \internal

    Connect to the reply and use the protocol to send it.
*/
bool QCoapClientPrivate::send(QCoapReply *reply)
{
    Q_Q(QCoapClient);

    if (!QCoapRequest::isUrlValid(reply->request().url())) {
        qWarning() << "QCoapClient: Failed to send request for an invalid URL.";
        return false;
    }

    q->connect(reply, SIGNAL(aborted(const QCoapToken&)),
               protocol, SLOT(onRequestAborted(const QCoapToken&)));

    QMetaObject::invokeMethod(protocol, "sendRequest", Qt::QueuedConnection,
                              Q_ARG(QPointer<QCoapReply>, QPointer<QCoapReply>(reply)),
                              Q_ARG(QCoapConnection*, connection));

    return true;
}

/*!
    Sets the maximum block size used by the protocol when sending requests
    and receiving replies. The block size must be a power of two.
*/
void QCoapClient::setBlockSize(quint16 blockSize)
{
    Q_D(QCoapClient);

    QMetaObject::invokeMethod(d->protocol, "setBlockSize", Qt::QueuedConnection,
                              Q_ARG(quint16, blockSize));
}

/*!
    Sets the QUdpSocket socket \a option to \a value.
*/
void QCoapClient::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
    Q_D(QCoapClient);

    QMetaObject::invokeMethod(d->connection, "setSocketOption", Qt::QueuedConnection,
                              Q_ARG(QAbstractSocket::SocketOption, option),
                              Q_ARG(QVariant, value));
}

#if 0
//! Disabled until fully supported
/*!
    Sets the protocol used by the client. Allows use of a custom protocol.
*/
void QCoapClient::setProtocol(QCoapProtocol *protocol)
{
    Q_D(QCoapClient);
    // FIXME: Protocol running on incorrect thread
    d->protocol = protocol;
}
#endif
QT_END_NAMESPACE
