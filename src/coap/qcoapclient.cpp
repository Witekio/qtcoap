#include <QEventLoop>
#include "qcoapclient.h"
#include "qcoapclient_p.h"

QT_BEGIN_NAMESPACE

QCoapClientPrivate::QCoapClientPrivate() :
    protocol(new QCoapProtocol),
    connection(new QCoapConnection),
    workerThread(new QThread)
{
    protocol->moveToThread(workerThread);
    connection->moveToThread(workerThread);
    workerThread->start();
}

QCoapClientPrivate::~QCoapClientPrivate()
{
}

/*!
    \class QCoapClient
    \brief The QCoapClient class allows the application to
    send coap requests and receive replies.

    \reentrant

    The QCoapClient class contains signals that can be used to know if the
    reply of a sent request arrived.

    The application can use a QCoapClient to send requests over a CoAP
    network. It contains functions for standard request and each function
    returns a QCoapReply object that stores the response data to a request
    and can be used to read these data.

    A simple request can be sent with :
    // TODO : replace by snippet
    \code
        QCoapClient* client = new QCoapClient(this);
        connect(client, &QCoapClient::finished, this, &TestClass::slotFinished);
        client->get(QCoapRequest(Qurl("coap://coap.me/test")));
    \endcode

    For an "observe" request, you can use it the same way than previously
    but using the \l{QCoapReply::notified(const QByteArray&)}
    {notified(const QByteArray&)} signal of the reply can be more useful.
    \code
        QCoapRequest request = QCoapRequest(Qurl("coap://coap.me/obs"));
        CoapReply* reply = client->observe(request);
        connect(reply, &QCoapReply::notified, this, &TestClass::slotNotified);
    \endcode

    And the observation can be cancelled with :
    \code
        client->cancelObserve(request);
    \endcode
    or
    \code
        client->cancelObserve(reply);
    \endcode

    When a reply arrive, the QCoapClient emits a
    \l{QCoapClient::finished(QCoapReply*)}{finished(QCoapReply*)} signal.

    \note For discovery request, the returned object is a QCoapDiscoveryReply.
    It can be used the same way as a QCoapReply but contains also a list of
    resources.

    \sa QCoapRequest, QCoapReply, QCoapDiscoveryReply
*/

/*!
    \fn void QCoapClient::finished(QCoapReply *reply)

    This signal is emitted whenever a coap reply is
    finished. The \a reply parameter will contain a pointer to the
    reply that has just finished. This signal is emitted along with
    the \l{QCoapReply::finished()} signal.

    \sa QCoapReply::finished(), QCoapReply::error(QCoapReply::QCoapNetworkError)
*/

/*!
    Constructs a QCoapClient object and sets \a parent as the parent object.
*/
QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
    Q_D(QCoapClient);
    connect(d->workerThread, &QThread::finished,
            d->workerThread, &QThread::deleteLater);
    connect(d->connection, SIGNAL(readyRead(const QByteArray&)),
            d->protocol, SLOT(messageReceived(const QByteArray&)));
    connect(d->protocol, &QCoapProtocol::finished,
            this, &QCoapClient::finished);
    qRegisterMetaType<QCoapReply::QCoapNetworkError>();
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
    Posts a GET request to the target \a request and returns a new
    QCoapReply object which emits the \l{QCoapReply::finished()}{finished()}
    signal whenever the response arrives.

    \sa post(), put(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply* QCoapClient::get(const QCoapRequest& request)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : get()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(GetCoapOperation);

    QCoapReply* reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    Posts a PUT request sending the contents of the \a data byte array to the
    target \a request and returns a new QCoapReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response arrives.

    \sa get(), post(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply* QCoapClient::put(const QCoapRequest& request, const QByteArray& data)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : put()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(PutCoapOperation);
    copyRequest.setPayload(data);

    QCoapReply* reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    \overload

    Posts a PUT request sending the contents of the \a data device to the
    target \a request.

    \note The device has to be open and readable before calling this function.

    \sa get(), post(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply* QCoapClient::put(const QCoapRequest& request, QIODevice* device)
{
    if (!device->isSequential())
        device->seek(0);

    return put(request, device->readAll());
}

/*!
    Posts a POST request sending the contents of the \a data byte array to the
    target \a request and returns a new QCoapReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever the response arrives.

    \sa get(), put(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply* QCoapClient::post(const QCoapRequest& request, const QByteArray& data)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : post()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(PostCoapOperation);
    copyRequest.setPayload(data);

    QCoapReply* reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    \overload

    Posts a POST request sending the contents of the \a data device to the
    target \a request.

    \note The device has to be open and readable before calling this function.

    \sa get(), put(), deleteResource(), observe(), discover(), cancelObserve()
*/
QCoapReply* QCoapClient::post(const QCoapRequest& request, QIODevice* device)
{
    if (!device->isSequential())
        device->seek(0);

    return post(request, device->readAll());
}

/*!
    Sends a DELETE request to the target of \a request.

    \sa get(), put(), post(), observe(), discover(), cancelObserve()
 */
QCoapReply* QCoapClient::deleteResource(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    qDebug() << "QCoapClient : delete()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(DeleteCoapOperation);

    QCoapReply* reply = d->sendRequest(copyRequest);
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
QCoapDiscoveryReply* QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    Q_D(QCoapClient);
    QUrl discoveryUrl(url.toString().append(discoveryPath).toUtf8());

    QCoapRequest request(discoveryUrl);
    request.setOperation(GetCoapOperation);

    QCoapDiscoveryReply* reply = d->sendDiscovery(request);
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
QCoapReply* QCoapClient::observe(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::ObserveCoapOption);
    copyRequest.setObserve(true);

    QCoapReply* reply = nullptr;
    reply = get(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

/*!
    Sends a request to cancel the observation of the target \a request.

    \sa get(), post(), put(), deleteResource(), observe(), discover()
*/
void QCoapClient::cancelObserve(const QCoapRequest& request)
{
    Q_D(QCoapClient);
    d->protocol->cancelObserve(d->requestMap[request]);
}

/*!
    \overload

    Sends a request to cancel the observation of the target used by the
    reply \a notifiedReply

    \sa get(), post(), put(), deleteResource(), observe(), discover()
*/
void QCoapClient::cancelObserve(QCoapReply* notifiedReply)
{
    d_func()->protocol->cancelObserve(notifiedReply);
}

/*!
    \internal

    Sends the coap \a request to its own url and returns a new QCoapReply
    object which emits the \l{QCoapReply::finished()}{finished()} signal
    whenever the response arrives.
*/
QCoapReply* QCoapClientPrivate::sendRequest(const QCoapRequest& request)
{
    qDebug() << "QCoapClient::sendRequest()";
    Q_Q(QCoapClient);

    // Prepare the reply and send it
    QCoapReply* reply = new QCoapReply(q);
    reply->setRequest(request);

    protocol->sendRequest(reply, connection);

    return reply;
}

/*!
    \internal

    Sends the coap \a request to its own url and returns a
    new QCoapDiscoveryReply object which emits the
    \l{QCoapReply::finished()}{finished()} signal whenever
    the response arrives.
*/
QCoapDiscoveryReply* QCoapClientPrivate::sendDiscovery(const QCoapRequest& request)
{
    Q_Q(QCoapClient);

    // Prepare the reply and send it
    QCoapDiscoveryReply* reply = new QCoapDiscoveryReply(q);
    reply->setRequest(request);

    protocol->sendRequest(reply, connection);

    return reply;
}

/*!
    Returns a pointer to the protocol used by the client.

    \sa setProtocol()
*/
QCoapProtocol* QCoapClient::protocol() const
{
    return d_func()->protocol;
}

/*!
    Returns a pointer to the connection used by the client.
*/
QCoapConnection* QCoapClient::connection() const
{
    return d_func()->connection;
}

/*!
    Sets the maximum block size used by the protocol when sending requests
    and receiving replies.
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
    QUdpSocket* udpSocket = d->connection->socket();
    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, ttlValue);
}

/*!
    Enables the loopback option for multicast requests.
*/
void QCoapClient::enableMulticastLoopbackOption()
{
    Q_D(QCoapClient);
    QUdpSocket* udpSocket = d->connection->socket();
    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
}

/*!
    Sets the protocol used by the client. Allow the user to make its
    own protocol class.

    \sa protocol()
*/
void QCoapClient::setProtocol(QCoapProtocol* protocol)
{
    Q_D(QCoapClient);
    d->protocol = protocol;
}

/*!
    \internal

    Sets the connection used by the client.

    \sa connection()
*/
void QCoapClientPrivate::setConnection(QCoapConnection* newConnection)
{
    connection = newConnection;
}

QT_END_NAMESPACE

#include "moc_qcoapclient.cpp"
