#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include <QNetworkDatagram>

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host_p(""),
    udpSocket_p(new QUdpSocket()),
    currentPdu_p(QByteArray()),
    port_p(0),
    state_p(QCoapConnection::UNCONNECTED)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
    udpSocket_p->disconnect();
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
    qDebug() << "Connect to host...";
    Q_D(QCoapConnection);

    QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket_p);

    connect(socket, SIGNAL(connected()), this, SLOT(_q_connectedToHost()));
    connect(socket, SIGNAL(readyRead()), this, SIGNAL(readyRead()));

    socket->connectToHost(d->host_p, d->port_p);
}

#include <QThread>
void QCoapConnection::sendRequest(const QByteArray& request)
{
    qDebug() << "Send request...";
    Q_D(QCoapConnection);

    d->currentPdu_p = request;
    // TODO : finish sendRequest
    if (d->state_p == UNCONNECTED) {
        connect(this, SIGNAL(connected()), this, SLOT(startToSendRequest()));
        connectToHost();
    }

    // TODO : slot instead (because of the connection that may take some times)
    if (d->state_p == CONNECTED) {
        qDebug() << "Gonna write";
        writeToSocket(request);
    }
}

QByteArray QCoapConnection::readReply()
{
    Q_D(QCoapConnection);

    //QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket_p);

    if (!d->udpSocket_p->isOpen())
        qDebug() << "open read : " << d->udpSocket_p->open(QIODevice::ReadWrite); // NOTE : change to read only ?

    if (!d->udpSocket_p->isSequential())
        d->udpSocket_p->seek(0);

    QByteArray reply;
    reply = d->udpSocket_p->readAll();
    return reply;
}

void QCoapConnection::writeToSocket(const QByteArray& data)
{
    Q_D(QCoapConnection);

    if (!d->udpSocket_p->isOpen())
        d->udpSocket_p->open(QIODevice::ReadWrite); // NOTE : change to write only ?*/

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

void QCoapConnection::startToSendRequest()
{
    qDebug() << "Gonna write";
    writeToSocket(d_func()->currentPdu_p);
}

void QCoapConnection::_q_connectedToHost()
{
    qDebug() << "CONNECTED";
    Q_D(QCoapConnection);
    d->state_p = QCoapConnection::CONNECTED;
    emit connected();
}
