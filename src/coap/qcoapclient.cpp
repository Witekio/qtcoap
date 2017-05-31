#include "qcoapclient.h"
#include "qcoapclient_p.h"

QCoapClientPrivate::QCoapClientPrivate()
{
}

QCoapClient::QCoapClient(QObject* parent) :
    QObject(* new QCoapClientPrivate, parent)
{
}

QCoapReply* QCoapClient::get(QCoapRequest* request)
{
    // TODO
    Q_UNUSED(request);
    return nullptr;
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

bool QCoapClient::containsMessageId(quint16 id)
{
    Q_D(QCoapClient);
    for (QCoapRequest* request : d->requests) {
        if (request->messageId() == id)
            return true;
    }

    return false;
}

void QCoapClient::sendRequest(QCoapRequest* request)
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
