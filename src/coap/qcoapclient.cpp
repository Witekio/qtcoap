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

QCoapReply* QCoapClient::get(QCoapRequest* request, QCoapRequest::QCoapMessageType type)
{
    qDebug() << "QCoapClient : get()";

    connect(request, SIGNAL(finished(QCoapRequest*)), this, SLOT(_q_requestFinished(QCoapRequest*)));

    request->setOperation(QCoapRequest::GET);
    request->setType(type);
    sendRequest(request);

    return request->reply();
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
