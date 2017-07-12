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

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
    Q_D(QCoapClient);
    connect(d->workerThread, &QThread::finished,
            d->workerThread, &QThread::deleteLater);
    connect(d->connection, &QCoapConnection::readyRead,
            d->protocol, &QCoapProtocol::messageReceived);
    qRegisterMetaType<QCoapReply::QCoapNetworkError>();
}

QCoapClient::~QCoapClient()
{
    Q_D(QCoapClient);
    d->workerThread->quit();
    d->workerThread->wait();
    delete d->workerThread;
    delete d->protocol;
    delete d->connection;
}

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
    copyRequest.setOperation(PostCoapOperation);
    copyRequest.setPayload(data);

    QCoapReply* reply = d->sendRequest(copyRequest);
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
    copyRequest.setOperation(DeleteCoapOperation);

    QCoapReply* reply = d->sendRequest(copyRequest);
    d->requestMap[request] = reply;

    return reply;
}

QCoapDiscoveryReply* QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    Q_D(QCoapClient);
    QUrl discoveryUrl(url.toString().append(discoveryPath));

    QCoapRequest request(discoveryUrl);
    request.setOperation(GetCoapOperation);

    QCoapDiscoveryReply* reply = d->sendDiscovery(request);
    d->requestMap[request] = reply;

    return reply;
}

QCoapReply* QCoapClient::observe(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::ObserveOption);
    copyRequest.setObserve(true);

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

QCoapReply* QCoapClientPrivate::sendRequest(const QCoapRequest& request)
{
    qDebug() << "QCoapClient::sendRequest()";
    Q_Q(QCoapClient);

    // Prepare the reply and send it
    QCoapReply* reply = new QCoapReply(q);
    reply->setRequest(request);

    qDebug() << "client : " << q->thread() << " protocol :" << protocol->thread();

    protocol->sendRequest(reply, connection);

    return reply;
}

QCoapDiscoveryReply* QCoapClientPrivate::sendDiscovery(const QCoapRequest& request)
{
    Q_Q(QCoapClient);

    // Prepare the reply and send it
    QCoapDiscoveryReply* reply = new QCoapDiscoveryReply(q);
    reply->setRequest(request);

    protocol->sendRequest(reply, connection);

    return reply;
}

QCoapProtocol* QCoapClient::protocol() const
{
    return d_func()->protocol;
}

QCoapConnection* QCoapClient::connection() const
{
    return d_func()->connection;
}

void QCoapClient::setBlockSize(quint16 blockSize)
{
    Q_D(QCoapClient);
    // If it is not a power of two
    if ((blockSize & (blockSize-1)) != 0)
        return;

    d->protocol->setBlockSize(blockSize);
}

void QCoapClient::setProtocol(QCoapProtocol* protocol)
{
    Q_D(QCoapClient);
    d->protocol = protocol;
}

QT_END_NAMESPACE

#include "moc_qcoapclient.cpp"
