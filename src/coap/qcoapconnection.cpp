#include "qcoapconnection.h"
#include "qcoapconnection_p.h"

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host_p(""),
    udpSocket_p(new QUdpSocket()),
    port_p(0),
    state_p(QCoapConnection::UNCONNECTED)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
    delete udpSocket_p;
}

QCoapConnection::QCoapConnection(const QString& host, int port, QObject* parent) :
    QObject(*new QCoapConnectionPrivate, parent)
{
    Q_D(QCoapConnection);
    d->host_p = host;
    d->port_p = port;
}

QCoapConnection::QCoapConnection(QCoapConnectionPrivate& dd, const QString& host, int port, QObject* parent) :
    QObject(dd, parent)
{
    Q_D(QCoapConnection);
    d->host_p = host;
    d->port_p = port;
}


void QCoapConnection::connectToHost()
{
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
    return d_func()->udpSocket_p;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state_p;
}
