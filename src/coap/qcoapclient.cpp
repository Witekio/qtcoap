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

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
    qRegisterMetaType<QCoapInternalReply>();
}

QCoapClient::~QCoapClient()
{
    Q_D(QCoapClient);
    qDeleteAll(d->resources);
    d->resources.clear();
    delete d->protocol;
}

QCoapReply* QCoapClient::get(const QCoapRequest& request)
{
    qDebug() << "QCoapClient : get()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(GET);

    QCoapReply* reply = sendRequest(copyRequest);

    return reply;
}

QCoapReply* QCoapClient::put(const QCoapRequest& request, const QByteArray& data)
{
    qDebug() << "QCoapClient : put()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(PUT);
    copyRequest.setPayload(data);

    QCoapReply* reply = sendRequest(copyRequest);

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
    qDebug() << "QCoapClient : post()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(POST);
    copyRequest.setPayload(data);

    QCoapReply* reply = sendRequest(copyRequest);

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
    qDebug() << "QCoapClient : delete()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(DELETE);

    QCoapReply* reply = sendRequest(copyRequest);

    return reply;
}

QCoapDiscoveryReply* QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    QUrl discoveryUrl(url.toString().append(discoveryPath));

    QCoapRequest request(discoveryUrl);
    request.setOperation(GET);

    QCoapDiscoveryReply* reply = sendDiscovery(request);

    return reply;
}

QCoapReply* QCoapClient::observe(const QCoapRequest& request)
{
    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::OBSERVE);
    copyRequest.setObserve(true);
    copyRequest.setType(QCoapMessage::CONFIRMABLE);
    QCoapReply* reply = nullptr;
    reply = get(copyRequest);

    return reply;
}

void QCoapClient::cancelObserve(QCoapReply* notifiedReply)
{
    d_func()->protocol->cancelObserve(notifiedReply);
}

QCoapConnection* QCoapClient::findConnection(QString host, int port)
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
    QCoapConnection* connection = findConnection(request.url().host(), request.url().port());
    if (!connection)
        connection = addConnection(request.url().host(), request.url().port());

    // Prepare the reply and send it
    QCoapReply* reply = new QCoapReply();
    reply->setRequest(request);
    //reply->moveToThread(workerThread); // To use the parameter in the signals/slots

    d->requestMap[request] = reply;
    d->protocol->sendRequest(reply, connection);

    //connect(workerThread, SIGNAL(started()), d->protocol, SLOT(startToSend()));
    //connect(reply, SIGNAL(finished()), workerThread, SLOT(quit()));
    //connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));

    //workerThread->start();
    //request.sendRequest();

    return reply;
}

QCoapDiscoveryReply* QCoapClient::sendDiscovery(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    // Find the connection or create a new connection
    QCoapConnection* connection = findConnection(request.url().host(), request.url().port());
    if (!connection)
        connection = addConnection(request.url().host(), request.url().port());

    // Prepare the reply and send it
    QCoapDiscoveryReply* reply = new QCoapDiscoveryReply();
    reply->setRequest(request);

    d->requestMap[request] = reply;
    d->protocol->sendRequest(reply, connection);

    return reply;
}

QCoapConnection* QCoapClient::addConnection(const QString& host, int port)
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
