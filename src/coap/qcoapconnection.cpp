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
    if (udpSocket_p->isOpen())
        udpSocket_p->close();
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
    Q_D(QCoapConnection);
    QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket_p);

    connect(socket, SIGNAL(connected()), this, SLOT(_q_connectedToHost()));

    socket->connectToHost(d->host_p, d->port_p);
}

#include <QThread> // TODO : remove this (this is just for test)
void QCoapConnection::sendRequest(const QByteArray& request)
{
    Q_UNUSED(request);
    Q_D(QCoapConnection);

    // TODO : finish sendRequest
    if (d->state_p == UNCONNECTED)
        connectToHost();

    //QThread::sleep(5);
    if (d->state_p == CONNECTED) {
        qDebug() << "connected";
        qDebug() << "opened : " << d->udpSocket_p->isOpen();
        writeToSocket(request);
    }
}

QByteArray QCoapConnection::readReply()
{
    Q_D(QCoapConnection);

    if (!d->udpSocket_p->isOpen())
        qDebug() << "open read : " << d->udpSocket_p->open(QIODevice::ReadWrite); // NOTE : change to read only ?

    d->udpSocket_p->seek(0);

    QByteArray reply;
    reply = d->udpSocket_p->readAll();

    return reply;
}

void QCoapConnection::writeToSocket(const QByteArray& data)
{
    Q_D(QCoapConnection);

    if (!d->udpSocket_p->isOpen())
        d->udpSocket_p->open(QIODevice::ReadWrite); // NOTE : change to write only ?

    d->udpSocket_p->write(data);
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

void QCoapConnection::setSocket(QIODevice* device)
{
    Q_D(QCoapConnection);
    d->udpSocket_p = device;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state_p;
}

void QCoapConnection::_q_connectedToHost()
{
    Q_D(QCoapConnection);
    d->state_p = QCoapConnection::CONNECTED;
    qDebug() << "SLOT CONNECTED";
    emit connected();
}

