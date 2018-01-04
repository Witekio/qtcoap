#include "coaphandler.h"

#include <QDebug>
#include <QCoapClient>
#include <QCoapReply>
#include <QCoapDiscoveryReply>

CoapHandler::CoapHandler(QObject *parent) : QObject(parent)
{
    qDebug() << "Started...";
    connect(&m_coapClient, &QCoapClient::error, this, &CoapHandler::onError);

    QCoapReply *getReply = m_coapClient.get(QUrl("coap://172.17.0.3/test"));
    connect(getReply, &QCoapReply::finished, this, &CoapHandler::onGetFinished);

    QCoapReply *observeReply = m_coapClient.observe(QUrl("coap://172.17.0.3/obs"));
    connect(observeReply, &QCoapReply::notified, this, &CoapHandler::onNotified);

    QCoapDiscoveryReply *discoveryReply = m_coapClient.discover(QUrl("coap://172.17.0.3"));
    connect(discoveryReply, &QCoapDiscoveryReply::discovered, this, &CoapHandler::onDiscovered);
}

void CoapHandler::onGetFinished(QCoapReply *reply)
{
    if (reply->errorReceived() != QtCoap::NoError)
        return;

    qDebug() << "Received GET answer: " << reply->readAll();
}

void CoapHandler::onNotified(QCoapReply *reply, QCoapMessage message)
{
    Q_UNUSED(message)
    qDebug() << "Received OBSERVE notification: " << reply->readAll();
    // You can alternatively use `message.payload();`
}

void CoapHandler::onDiscovered(QVector<QCoapResource> resources, QCoapDiscoveryReply *reply)
{
    Q_UNUSED(reply)

    foreach(const QCoapResource res, resources) {
        qDebug() << "Resource discovered: " << res.path() << res.title();
    }
}

void CoapHandler::onError(QCoapReply *reply, QtCoap::Error error)
{
    if (reply)
        qDebug() << "CoAP reply error: " << reply->errorString();
    else
        qDebug() << "CoAP error: " << error;
}
