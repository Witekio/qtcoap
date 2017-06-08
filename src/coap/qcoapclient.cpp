#include "qcoapclient.h"
#include "qcoapclient_p.h"

QCoapClientPrivate::QCoapClientPrivate()
{
}

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
}

QCoapReply* QCoapClient::get(QCoapRequest* request, QCoapRequest::QCoapMessageType type)
{
    qDebug() << "QCoapClient : get()";

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(requestFinished(QCoapRequest*)));

    request->setOperation(QCoapRequest::GET);
    request->setType(type);
    sendRequest(request);

    return request->reply();
}

bool QCoapClient::containsToken(QByteArray token)
{
    Q_D(QCoapClient);
    for (QCoapRequest* request : d->requests) {
        if (request->token() == token)
            return true;
    }

    return false;
}

int QCoapClient::findRequestByToken(QByteArray token)
{
    Q_D(QCoapClient);
    int id = 0;
    for (QCoapRequest* request : d->requests) {
        if (request->token() == token)
            return id;
        ++id;
    }

    return -1;
}

bool QCoapClient::containsMessageId(quint16 id)
{
    Q_D(QCoapClient);
    for (QCoapRequest* request : d->requests) {
        if (request->messageId() == id)
            return true;
    }

    return false;
}

void QCoapClient::addRequest(QCoapRequest* request)
{
    Q_D(QCoapClient);

    QByteArray token = request->token();
    containsToken(token);
    token = request->generateToken();
    while (token == QByteArray() || containsToken(token))
           token = request->generateToken();
    request->setToken(token);

    quint64 messageId = request->messageId();
    while (messageId == 0 || containsMessageId(messageId))
           messageId = request->generateMessageId();
    request->setMessageId(messageId);

    d->requests.push_back(request);
}

void QCoapClient::sendRequest(QCoapRequest* request)
{
    addRequest(request);
    request->sendRequest();
}

void QCoapClient::requestFinished(QCoapRequest* request)
{
    Q_D(QCoapClient);
    int idRequest = findRequestByToken(request->token());
    if (idRequest != -1)
        d->requests.removeAt(idRequest);

    emit finished();
}
