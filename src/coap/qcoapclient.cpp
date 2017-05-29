#include "qcoapclient.h"
#include "qcoapclient_p.h"

QCoapClientPrivate::QCoapClientPrivate()
{
}

QCoapClient::QCoapClient(QObject* parent) :
    QObject(parent)
{
}

QCoapReply* QCoapClient::get(const QCoapRequest& request)
{
    // TODO
    Q_UNUSED(request);
    return nullptr;
}
