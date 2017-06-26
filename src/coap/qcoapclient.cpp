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

QCoapReply* QCoapClient::post(const QCoapRequest& request, const QByteArray& data)
{
    qDebug() << "QCoapClient : post()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(POST);
    copyRequest.setPayload(data);

    QCoapReply* reply = sendRequest(copyRequest);

    return reply;
}

QCoapReply* QCoapClient::deleteResource(const QCoapRequest& request)
{
    qDebug() << "QCoapClient : delete()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(DELETE);

    QCoapReply* reply = sendRequest(copyRequest);

    return reply;
}

QList<QCoapResource*> QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    Q_D(QCoapClient);
    // NOTE : Block or not ? (Send a signal instead and a pointer to the list ?)
    // (Return the reply and let the use parse himself if he wants ?)
    QEventLoop loop;

    QUrl discoveryUrl(url.toString().append(discoveryPath));

    QCoapReply* reply = nullptr;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    reply = get(QCoapRequest(discoveryUrl));

    loop.exec();

    d->resources = QCoapResource::fromCoreLinkList(reply->readAll());

    return d->resources;
}

QCoapReply* QCoapClient::observe(const QCoapRequest& request)
{
    QCoapRequest copyRequest(request);
    copyRequest.addOption(QCoapOption::OBSERVE, QByteArray(""));
    copyRequest.setObserve(true);
    copyRequest.setType(QCoapMessage::CONFIRMABLE);
    QCoapReply* reply = get(copyRequest);

    return reply;
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

    QThread *workerThread = d->workerThread;

    // Find the connection or create a new connection
    QCoapConnection* connection = findConnection(request.url().host(), request.url().port());
    if (!connection) {
        connection = new QCoapConnection(request.url().host(), request.url().port());
        connection->connectToHost();
        d->connections.push_back(connection);
        /*connect(connection, SIGNAL(readyRead(const QByteArray&)),
                d->protocol, SLOT(messageReceived(const QByteArray&)));*/
        connection->moveToThread(workerThread);
        //connection->socket()->moveToThread(workerThread); // The socket is not directly a child of connection
    }

    // Prepare the reply and send it
    QCoapReply* reply = new QCoapReply();
    reply->setRequest(request);
    //reply->moveToThread(workerThread); // To use the parameter in the signals/slots

    d->requestsMap[request] = reply;
    d->protocol->sendRequest(reply, connection);

    //connect(workerThread, SIGNAL(started()), d->protocol, SLOT(startToSend()));
    //connect(reply, SIGNAL(finished()), workerThread, SLOT(quit()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));

    /*connect(d->protocol, SIGNAL(lastBlockReceived(const QCoapInternalReply&)),
                     reply, SLOT(updateWithInternalReply(const QCoapInternalReply&)));*/

    //workerThread->start();
    //request.sendRequest();

    return reply;
}

QT_END_NAMESPACE

#include "moc_qcoapclient.cpp"
