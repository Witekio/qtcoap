#include <QEventLoop>
#include "qcoapclient.h"
#include "qcoapclient_p.h"

QT_BEGIN_NAMESPACE

QCoapClientPrivate::QCoapClientPrivate()
{
}

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
}

QCoapClient::~QCoapClient()
{
    Q_D(QCoapClient);
    qDeleteAll(d->resources);
    d->resources.clear();
}

QCoapReply* QCoapClient::get(const QCoapRequest& request)
{
    qDebug() << "QCoapClient : get()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(GET);

    sendRequest(copyRequest);

    return copyRequest.reply();
}

QCoapReply* QCoapClient::put(const QCoapRequest& request, const QByteArray& data)
{
    qDebug() << "QCoapClient : put()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(PUT);
    copyRequest.setPayload(data);

    sendRequest(copyRequest);

    return copyRequest.reply();
}

QCoapReply* QCoapClient::post(const QCoapRequest& request, const QByteArray& data)
{
    qDebug() << "QCoapClient : post()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(POST);
    copyRequest.setPayload(data);

    sendRequest(copyRequest);

    return copyRequest.reply();
}

QCoapReply* QCoapClient::deleteResource(const QCoapRequest& request)
{
    qDebug() << "QCoapClient : delete()";

    QCoapRequest copyRequest(request);
    copyRequest.setOperation(DELETE);

    sendRequest(copyRequest);

    return copyRequest.reply();
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

bool QCoapClientPrivate::containsToken(QByteArray token)
{
    for (QCoapRequest* request : requests) {
        if (request->token() == token)
            return true;
    }

    return false;
}

int QCoapClientPrivate::findRequestByToken(QByteArray token)
{
    int id = 0;
    for (QCoapRequest* request : requests) {
        if (request->token() == token)
            return id;
        ++id;
    }

    return -1;
}

bool QCoapClientPrivate::containsMessageId(quint16 id)
{
    for (QCoapRequest* request : requests) {
        if (request->messageId() == id)
            return true;
    }

    return false;
}

void QCoapClient::addRequest(const QCoapRequest& request)
{
    Q_D(QCoapClient);

    // TODO : put the generation of tokens in QCoapRequest
    /*QByteArray token = request.token();
    d->containsToken(token);
    token = request.generateToken();
    while (token == QByteArray() || d->containsToken(token))
           token = request->generateToken();
    request->setToken(token);

    quint16 messageId = request->messageId();
    while (messageId == 0 || d->containsMessageId(messageId))
           messageId = request->generateMessageId();
    request->setMessageId(messageId);*/

    //d->requests.push_back(request);
}

void QCoapClient::sendRequest(const QCoapRequest& request)
{
    addRequest(request);
    request.sendRequest();
}

void QCoapClientPrivate::_q_requestFinished(QCoapRequest* request)
{
    Q_Q(QCoapClient);

    int idRequest = findRequestByToken(request->token());
    if (idRequest != -1)
        requests.removeAt(idRequest);

    emit q->finished();
}

QT_END_NAMESPACE

#include "moc_qcoapclient.cpp"
