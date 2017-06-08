#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include <QNetworkDatagram>

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host(""),
    udpSocket(new QUdpSocket()),
    currentPdu(QByteArray()),
    port(0),
    state(QCoapConnection::UNCONNECTED),
    sendingState(QCoapConnection::WAITING)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
    udpSocket->disconnect();
    if (udpSocket->isOpen())
        udpSocket->close();
    delete udpSocket;
}

QCoapConnection::QCoapConnection(const QString& host, int port, QObject* parent) :
    QCoapConnection(*new QCoapConnectionPrivate, host, port, parent)
{
}

QCoapConnection::QCoapConnection(QCoapConnectionPrivate& dd, const QString& host, int port, QObject* parent) :
    QObject(dd, parent)
{
    Q_D(QCoapConnection);
    d->host = host;
    d->port = port;
}

void QCoapConnection::connectToHost()
{
    Q_D(QCoapConnection);

    /*if (d->udpSocket == nullptr)
        d->udpSocket = new QUdpSocket();*/

    QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket);

    connect(socket, SIGNAL(connected()), this, SLOT(_q_connectedToHost()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(_q_socketReadyRead()));

    socket->connectToHost(d->host, d->port);
}

void QCoapConnection::sendRequest(const QByteArray& request)
{
    Q_D(QCoapConnection);

    d->currentPdu = request;
    if (d->state == UNCONNECTED) {
        qDebug() << "QCoapConnection : sendRequest() - UNCONNECTED";
        connect(this, SIGNAL(connected()), this, SLOT(startToSendRequest()));
        connectToHost();
    } else if (d->state == CONNECTED) {
        qDebug() << "QCoapConnection : sendRequest() - CONNECTED";
        writeToSocket(request);
    }
}

QByteArray QCoapConnection::readReply()
{
    Q_D(QCoapConnection);

    if (!d->udpSocket->isOpen())
        d->udpSocket->open(QIODevice::ReadWrite); // NOTE : change to read only ?

    // Because for the tests we use a QBuffer that is not sequential
    if (!d->udpSocket->isSequential())
        d->udpSocket->seek(0);

    QByteArray reply;
    reply = d->udpSocket->readAll();
    qDebug() << "QCoapConnection::readReply() - " << reply;
    return reply;
}

void QCoapConnection::writeToSocket(const QByteArray& data)
{
    Q_D(QCoapConnection);

    if (!d->udpSocket->isOpen())
        d->udpSocket->open(QIODevice::ReadWrite); // NOTE : change to write only ?

    qDebug() << "QCoapConnection::writeToSocket()";
    d->udpSocket->write(data);
}

QString QCoapConnection::host() const
{
    return d_func()->host;
}

void QCoapConnection::setHost(const QString& host)
{
    Q_D(QCoapConnection);
    if (d->host == host)
        return;

    d->host = host;
}

void QCoapConnection::setPort(int port)
{
    Q_D(QCoapConnection);
    if (d->port == port)
        return;

    d->port = port;
}

int QCoapConnection::port() const
{
    return d_func()->port;
}

QIODevice* QCoapConnection::socket() const
{
    return d_func()->udpSocket;
}

void QCoapConnection::setSocket(QIODevice* device)
{
    Q_D(QCoapConnection);
    if (d->udpSocket == device)
        return;

    d->udpSocket = device;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state;
}

void QCoapConnection::startToSendRequest()
{
    Q_D(QCoapConnection);

    if (d->sendingState == WAITING) {
        d->sendingState = SENDING;
        writeToSocket(d->currentPdu);
    }
}

void QCoapConnection::_q_connectedToHost()
{
    Q_D(QCoapConnection);

    if (d->state == CONNECTED)
        return;

     d->state = QCoapConnection::CONNECTED;
     qDebug() << "emit connected()";
     emit connected();
}

void QCoapConnection::_q_socketReadyRead()
{
    Q_D(QCoapConnection);

    if (d->sendingState == COMPLETE)
        return;

    d->sendingState = COMPLETE;
    emit readyRead();
}
