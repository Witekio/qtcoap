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

QCoapReply* QCoapClient::get(QCoapRequest* request)
{
    qDebug() << "QCoapClient : get()";

    request->setOperation(QCoapRequest::GET);

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    sendRequest(request);

    return request->reply();
}

QCoapReply* QCoapClient::put(QCoapRequest* request, const QByteArray& data)
{
    qDebug() << "QCoapClient : put()";

    request->setOperation(QCoapRequest::PUT);
    request->setPayload(data);

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    sendRequest(request);

    return request->reply();
}

QCoapReply* QCoapClient::post(QCoapRequest* request, const QByteArray& data)
{
    qDebug() << "QCoapClient : post()";

    request->setOperation(QCoapRequest::POST);
    request->setPayload(data);

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    sendRequest(request);

    return request->reply();
}

QCoapReply* QCoapClient::deleteResource(QCoapRequest* request)
{
    qDebug() << "QCoapClient : delete()";

    request->setOperation(QCoapRequest::DELETE);

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    sendRequest(request);

    return request->reply();
}

QCoapReply* QCoapClient::discover(const QUrl& url, const QString& discoveryPath)
{
    QUrl discoveryUrl(url.toString().append(discoveryPath));

    QCoapRequest* request = new QCoapRequest(discoveryUrl);
    request->setOperation(QCoapRequest::GET);

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    sendRequest(request);

    QCoapReply* reply = request->reply();

    // TODO : delete request;
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

void QCoapClient::addRequest(QCoapRequest* request)
{
    Q_D(QCoapClient);

    // TODO : put the generation of tokens in QCoapRequest
    QByteArray token = request->token();
    d->containsToken(token);
    token = request->generateToken();
    while (token == QByteArray() || d->containsToken(token))
           token = request->generateToken();
    request->setToken(token);

    quint16 messageId = request->messageId();
    while (messageId == 0 || d->containsMessageId(messageId))
           messageId = request->generateMessageId();
    request->setMessageId(messageId);

    d->requests.push_back(request);
}

void QCoapClient::sendRequest(QCoapRequest* request)
{
    addRequest(request);
    request->sendRequest();
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
