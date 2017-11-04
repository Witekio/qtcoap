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

#include "qcoapclient_p.h"
#include "qcoapreply.h"
#include "qcoapdiscoveryreply.h"
#include <QtCore/qurl.h>
#include <QtNetwork/qudpsocket.h>

QT_BEGIN_NAMESPACE

QCoapClientPrivate::QCoapClientPrivate() :
    protocol(new QCoapProtocol),
    connection(new QCoapConnection),
    workerThread(new QThread)
{
    workerThread->start();
    protocol->moveToThread(workerThread);
    connection->moveToThread(workerThread);
}

QCoapClientPrivate::~QCoapClientPrivate()
{
}

/*!
    \class QCoapClient
    \brief The QCoapClient class allows the application to
    send coap requests and receive replies.

    \reentrant

    The QCoapClient class contains signals that gets triggered when the
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
        client->cancelObserve(request);
    \endcode
    or
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
    whenever a CoAP reply is finished. The \a reply parameter will contain a
    pointer to the reply that has just finished.

    \sa QCoapReply::finished(), QCoapReply::error(QCoapReply::QCoapNetworkError)
*/

/*!
    Constructs a QCoapClient object and sets \a parent as the parent object.
*/
QCoapClient::QCoapClient(QObject *parent) :
    QObject(* new QCoapClientPrivate, parent)
{
    Q_D(QCoapClient);
    connect(d->workerThread, &QThread::finished,
            d->workerThread, &QThread::deleteLater);
    connect(d->connection, SIGNAL(readyRead(const QByteArray&)),
            d->protocol, SLOT(messageReceived(const QByteArray&)));
    connect(d->protocol, &QCoapProtocol::finished,
            this, &QCoapClient::finished);
    qRegisterMetaType<QPointer<QCoapReply>>();
    qRegisterMetaType<QCoapReply*>();
    qRegisterMetaType<QPointer<QCoapDiscoveryReply>>();
    qRegisterMetaType<QCoapConnection*>();
    qRegisterMetaType<QCoapReply::NetworkError>();
}

/*!
    Destroys the QCoapClient object and frees up any
    resources. Note that QCoapReply objects that are returned from
    this class have the QCoapClient set as their parents, which means that
    they will be deleted along with it.
*/
QCoapClient::~QCoapClient()
{
    Q_D(QCoapClient);
    d->workerThread->quit();
    d->workerThread->wait();
    delete d->workerThread;
    delete d->protocol;
    delete d->connection;
    qDeleteAll(findChildren<QCoapReply*>(QString(), Qt::FindDirectChildrenOnly));
}

/*!
    Posts a GET request to \a target and returns a new
    QCoapReply object which emits the \l{QCoapReply::finished()}{finished()}
    signal whenever the response arrives.

    \sa post(), put(), deleteResource(), observe(), discover()
*/
QCoapReply *QCoapClient::get(const QCoapRequest &target)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(target);
    copyRequest.setOperation(QCoapRequest::Get);

    QCoapReply *reply = d->sendRequest(copyRequest);
    d->requestMap[target] = reply;

    return reply;
}

/*!
    Posts a PUT request sending the contents of the \a data byte array to the
    target \a request and returns a new QCoapReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response arrives.

    \sa get(), post(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(QCoapRequest::Put);
    copyRequest.setPayload(data);

    QCoapReply *reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    \overload

    Posts a PUT request sending the contents of the \a data device to the
    target \a request. If the device is null, then it returns a nullptr.

    \note The device has to be open and readable before calling this function.

    \sa get(), post(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply *QCoapClient::put(const QCoapRequest &request, QIODevice *device)
{
     return put(request, device ? device->readAll() : QByteArray());
}

/*!
    Posts a POST request sending the contents of the \a data byte array to the
    target \a request and returns a new QCoapReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response arrives.

    \sa get(), put(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, const QByteArray &data)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(QCoapRequest::Post);
    copyRequest.setPayload(data);

    QCoapReply *reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    \overload

    Posts a POST request sending the contents of the \a data device to the
    target \a request. If the device is null, then it returns a nullptr.

    \note The device has to be open and readable before calling this function.

    \sa get(), put(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply *QCoapClient::post(const QCoapRequest &request, QIODevice *device)
{
    if (!device)
        return nullptr;

    return post(request, device->readAll());
}

/*!
    Sends a DELETE request to the target of \a request.

    \sa get(), put(), post(), observe(), discover(), cancelObserve()
 */
QCoapReply *QCoapClient::deleteResource(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(QCoapRequest::Delete);

    QCoapReply *reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    Discovers the resources available at the given \a url and returns
    a new QCoapDiscoveryReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response
    arrives.

    The path of the discovery can be set by giving the \a discoveryPath.

    \sa get(), post(), put(), deleteResource(), observe(), cancelObserve()
*/
QCoapDiscoveryReply *QCoapClient::discover(const QUrl &url, const QString &discoveryPath)
{
    Q_D(QCoapClient);

    QUrl discoveryUrl(url);
    discoveryUrl.setPath(url.path() + discoveryPath);

    QCoapRequest request(discoveryUrl);
    request.setOperation(QCoapRequest::Get);

    QCoapDiscoveryReply *reply = d->sendDiscovery(request);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    Sends a request to observe the target \a request and returns
    a new QCoapReply object which emits the
    \l{QCoapReply::notified(const QByteArray&)}{notified(const QByteArray&)}
    signal whenever a new notification arrives.

    \sa get(), post(), put(), deleteResource(), discover(), cancelObserve()
*/
QCoapReply *QCoapClient::observe(const QCoapRequest &request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::Observe);
    copyRequest.enableObserve();

    QCoapReply *reply = nullptr;
    reply = get(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    Sends a request to cancel the observation of the target \a request.

    \sa get(), post(), put(), deleteResource(), observe(), discover()
*/
void QCoapClient::cancelObserve(const QCoapRequest &request)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "cancelObserve",
                              Q_ARG(QPointer<QCoapReply>, d->requestMap[request]));
}

/*!
    \overload

    Sends a request to cancel the observation of the target used by the
    reply \a notifiedReply

    \sa get(), post(), put(), deleteResource(), observe(), discover()
*/
void QCoapClient::cancelObserve(QCoapReply *notifiedReply)
{
    Q_D(QCoapClient);
    QMetaObject::invokeMethod(d->protocol, "cancelObserve",
                              Q_ARG(QPointer<QCoapReply>, notifiedReply));
}

/*!
    \internal

    Sends the coap \a request to its own url and returns a new QCoapReply
    object which emits the \l{QCoapReply::finished()}{finished()} signal
    whenever the response arrives.
*/
QCoapReply *QCoapClientPrivate::sendRequest(const QCoapRequest &request)
{
    Q_Q(QCoapClient);

    QString scheme = request.url().scheme();
    if (scheme != QLatin1String("coap"))
        return nullptr;

    // Prepare the reply
    QPointer<QCoapReply> reply = new QCoapReply(q);
    reply->setRequest(request);

    // connect with DirectConnection type to secure from deleting the reply
    // (reply destructor emits the signal)
    q->connect(reply, SIGNAL(aborted(QCoapReply*)),
               protocol, SLOT(onAbortedRequest(QCoapReply*)), Qt::DirectConnection);
    q->connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
               reply, SLOT(connectionError(QAbstractSocket::SocketError)));

    QMetaObject::invokeMethod(protocol, "sendRequest",
                              Q_ARG(QPointer<QCoapReply>, reply), Q_ARG(QCoapConnection*, connection));

    return reply;
}

/*!
    \internal

    Sends the coap \a request to its own url and returns a
    new QCoapDiscoveryReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever
    the response arrives.
*/
QCoapDiscoveryReply *QCoapClientPrivate::sendDiscovery(const QCoapRequest &request)
{
    Q_Q(QCoapClient);

    QString scheme = request.url().scheme();
    if (scheme != QLatin1String("coap"))
        return nullptr;

    // Prepare the reply and send it
    QPointer<QCoapReply> reply = new QCoapDiscoveryReply(q);
    reply->setRequest(request);

    // connect with DirectConnection type to secure from deleting the reply
    // (reply destructor emits the signal)
    q->connect(reply, SIGNAL(aborted(QCoapReply*)),
               protocol, SLOT(onAbortedRequest(QCoapReply*)), Qt::DirectConnection);
    q->connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
               reply, SLOT(connectionError(QAbstractSocket::SocketError)));

    QMetaObject::invokeMethod(protocol, "sendRequest",
                              Q_ARG(QPointer<QCoapReply>, reply), Q_ARG(QCoapConnection*, connection));

    return static_cast<QCoapDiscoveryReply*>(reply.data());
}

/*!
    Sets the maximum block size used by the protocol when sending requests
    and receiving replies. The block size must be a power of two.
*/
void QCoapClient::setBlockSize(quint16 blockSize)
{
    Q_D(QCoapClient);
    // If it is not a power of two
    if ((blockSize & (blockSize-1)) != 0)
        return;

    d->protocol->setBlockSize(blockSize);
}

/*!
    Sets the ttl option for multicast requests.
*/
void QCoapClient::setMulticastTtlOption(int ttlValue)
{
    Q_D(QCoapClient);
    QUdpSocket *udpSocket = d->connection->socket();
    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, ttlValue);
}

/*!
    Enables the loopback option for multicast requests.
*/
void QCoapClient::enableMulticastLoopbackOption()
{
    Q_D(QCoapClient);
    QUdpSocket *udpSocket = d->connection->socket();
    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
}

/*!
    Sets the protocol used by the client. Allows the user to make its
    own protocol class.
*/
void QCoapClient::setProtocol(QCoapProtocol *protocol)
{
    Q_D(QCoapClient);
    d->protocol = protocol;
}

QT_END_NAMESPACE
