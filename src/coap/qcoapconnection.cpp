#include "qcoapconnection.h"
#include "qcoapconnection_p.h"

QCoapConnectionPrivate::QCoapConnectionPrivate()
{
}

QCoapConnection::QCoapConnection(const QString& host, int port, QObject* parent) :
    QObject(parent)
{
    Q_D(QCoapConnection);
    d->host_p = host;
    d->port_p = port;
}

void QCoapConnection::connectToHost() {
    // TODO
}

void QCoapConnection::sendRequest(const QByteArray& request)
{
    Q_UNUSED(request);
    // TODO
}

QByteArray QCoapConnection::readReply()
{
    // TODO
    return QByteArray();
}

QString QCoapConnection::host() const
{
    return d_func()->host_p;
}

int QCoapConnection::port() const
{
    return d_func()->port_p;
}

QIODevice* QCoapConnection::socket() const
{
    //return d_func()->udpSocket_p;
    return nullptr;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state_p;
}
