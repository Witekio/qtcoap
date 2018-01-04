#ifndef COAPHANDLER_H
#define COAPHANDLER_H

#include <QObject>
#include <QCoapClient>
#include <QCoapMessage>
#include <QCoapResource>
#include <qcoapnamespace.h>

class QCoapReply;
class QCoapDiscoveryReply;
class QCoapResource;

class CoapHandler : public QObject
{
    Q_OBJECT
public:
    explicit CoapHandler(QObject *parent = nullptr);

public Q_SLOTS:
    void onGetFinished(QCoapReply *reply);
    void onNotified(QCoapReply *reply, QCoapMessage message);
    void onDiscovered(QVector<QCoapResource> resources, QCoapDiscoveryReply *reply);
    void onError(QCoapReply *reply, QtCoap::Error error);

private:
    QCoapClient m_coapClient;
};

#endif // COAPHANDLER_H
