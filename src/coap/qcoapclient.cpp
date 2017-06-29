#include <QEventLoop>
#include "qcoapclient.h"
#include "qcoapclient_p.h"

QT_BEGIN_NAMESPACE

QCoapClientPrivate::QCoapClientPrivate() :
    protocol(new QCoapProtocol),
    workerThread(new QThread)
{
    protocol->moveToThread(workerThread);
    workerThread->start();
}

QCoapClientPrivate::~QCoapClientPrivate()
{
}

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
    Q_D(QCoapClient);
    connect(d->workerThread, SIGNAL(finished()), d->workerThread, SLOT(deleteLater()));
}

QCoapClient::~QCoapClient()
{
    Q_D(QCoapClient);
    d->workerThread->quit();
    d->workerThread->wait();
    delete d->workerThread;
    delete d->protocol;
    qDeleteAll(d->connections);
}

QCoapReply* QCoapClient::get(const QCoapRequest& request)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : get()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(GET);

    QCoapReply* reply = sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

QCoapReply* QCoapClient::put(const QCoapRequest& request, const QByteArray& data)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : put()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(PUT);
    copyRequest.setPayload(data);

    QCoapReply* reply = sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

QCoapReply* QCoapClient::put(const QCoapRequest& request, QIODevice* device)
{
    if (!device->isReadable())
        device->open(device->openMode() | QIODevice::ReadOnly);

    if (!device->isSequential())
        device->seek(0);

    return put(request, device->readAll());
}

QCoapReply* QCoapClient::post(const QCoapRequest& request, const QByteArray& data)
{
    Q_D(QCoapClient);
    qDebug() << "QCoapClient : post()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(POST);
    copyRequest.setPayload(data);

    QCoapReply* reply = sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

QCoapReply* QCoapClient::post(const QCoapRequest& request, QIODevice* device)
{
    if (!device->isReadable())
        device->open(device->openMode() | QIODevice::ReadOnly);

    if (!device->isSequential())
        device->seek(0);

    return post(request, device->readAll());
}

QCoapReply* QCoapClient::deleteResource(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    qDebug() << "QCoapClient : delete()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(DELETE);

    QCoapReply* reply = sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

QCoapDiscoveryReply* QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    Q_D(QCoapClient);
    QUrl discoveryUrl(url.toString().append(discoveryPath));

    QCoapRequest request(discoveryUrl);
    request.setOperation(GET);

    QCoapDiscoveryReply* reply = sendDiscovery(request);
    d->requestMap[request] = reply;

    return reply;
}

QCoapReply* QCoapClient::observe(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::OBSERVE);
    copyRequest.setObserve(true);
    copyRequest.setType(QCoapMessage::CONFIRMABLE);

    QCoapReply* reply = nullptr;
    reply = get(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

void QCoapClient::cancelObserve(const QCoapRequest& request)
{
    Q_D(QCoapClient);
    d->protocol->cancelObserve(d->requestMap[request]);
}

void QCoapClient::cancelObserve(QCoapReply* notifiedReply)
{
    d_func()->protocol->cancelObserve(notifiedReply);
}

QCoapConnection* QCoapClient::findConnection(QString host, quint16 port)
{
    for (QCoapConnection* connection : d_func()->connections) {
        if (connection->host() == host && connection->port() == port)
            return connection;
    }

    return nullptr;
}

QCoapReply* QCoapClient::sendRequest(const QCoapRequest& request)
{
    qDebug() << "QCoapClient::sendRequest()";
    Q_D(QCoapClient);

    // Find the connection or create a new connection
    //QCoapConnection* connection = findConnection(request.url().host(), request.url().port());
    //if (!connection)
        QCoapConnection* connection = addConnection(request.url().host(),
                                                    static_cast<quint16>(request.url().port()));

    // Prepare the reply and send it
    QCoapReply* reply = new QCoapReply();
    reply->setRequest(request);

    d->protocol->sendRequest(reply, connection);

    //connect(reply, SIGNAL(finished()), workerThread, SLOT(quit()));

    return reply;
}

QCoapDiscoveryReply* QCoapClient::sendDiscovery(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    // Find the connection or create a new connection
    //QCoapConnection* connection = findConnection(request.url().host(), request.url().port());
    //if (!connection)
        QCoapConnection* connection = addConnection(request.url().host(),
                                                    static_cast<quint16>(request.url().port()));

    // Prepare the reply and send it
    QCoapDiscoveryReply* reply = new QCoapDiscoveryReply();
    reply->setRequest(request);

    d->protocol->sendRequest(reply, connection);

    return reply;
}

QCoapConnection* QCoapClient::addConnection(const QString& host, quint16 port)
{
    Q_D(QCoapClient);

    QCoapConnection *connection;
    connection = new QCoapConnection(host, port);
    connection->connectToHost();
    d->connections.push_back(connection);
    connect(connection, SIGNAL(readyRead(const QByteArray&)),
            d->protocol, SLOT(messageReceived(const QByteArray&)));
    connection->moveToThread(d->workerThread);

    return connection;
}

QT_END_NAMESPACE

#include "moc_qcoapclient.cpp"
