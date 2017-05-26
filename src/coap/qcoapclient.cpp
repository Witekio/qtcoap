#include "qcoapclient.h"

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
