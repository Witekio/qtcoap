#include "qcoapconnection.h"
#include "qcoapconnection_p.h"

#include <QNetworkDatagram>

QT_BEGIN_NAMESPACE

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host(""),
    port(0),
    udpSocket(nullptr),
    currentPdu(QByteArray()),
    state(QCoapConnection::Unconnected)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
}

QCoapConnection::QCoapConnection(const QString& host, quint16 port, QObject* parent) :
    QCoapConnection(*new QCoapConnectionPrivate, host, port, parent)
{
}

QCoapConnection::QCoapConnection(QCoapConnectionPrivate& dd, const QString& host, quint16 port, QObject* parent) :
    QObject(dd, parent)
{
    Q_D(QCoapConnection);

    d->host = host;
    d->port = port;

    // Make the socket a child of the connection to move it in the same thread
    d->udpSocket = new QUdpSocket(this);
}

QCoapConnection::~QCoapConnection()
{
    Q_D(QCoapConnection);
    delete d->udpSocket;
}

void QCoapConnection::connectToHost()
{
    Q_D(QCoapConnection);

    //qDebug() << "QCoapConnection::connectToHost() - " << d->host << " - " << d->port;

    QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket);

    connect(socket, SIGNAL(connected()), this, SLOT(_q_connectedToHost()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(_q_disconnectedFromHost()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(_q_socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(_q_socketError(QAbstractSocket::SocketError)));

    socket->connectToHost(d->host, d->port);
}

void QCoapConnectionPrivate::bindToHost()
{
    qDebug() << "QCoapConnection::bindToHost()";

    Q_Q(QCoapConnection);

    QUdpSocket* socket = static_cast<QUdpSocket*>(udpSocket);
    if (socket->state() == QUdpSocket::ConnectedState)
        socket->disconnectFromHost();

    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), q, SLOT(_q_socketError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(readyRead()), q, SLOT(_q_socketReadyRead()));
    if (socket->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress)) {
        qDebug() << "QCoapConnection::bindToHost() - OK";
        qDebug() << "socket port : " << socket->localPort();
        boundToHost();
    }
}

void QCoapConnection::sendRequest(const QByteArray& request, const QString& host, quint16 port)
{
    //qDebug() << "QCoapConnection::sendRequest()";
    Q_D(QCoapConnection);

    //setSendingState(QCoapConnection::WAITING);

    d->currentPdu = request;
    d->host = host;
    d->port = port;

//    if (d->state == Unconnected) {
//        //qDebug() << "QCoapConnection : sendRequest() - Unconnected";
//        connect(this, SIGNAL(connected()), this, SLOT(_q_startToSendRequest()));
//        connectToHost();
//    } else if (d->state == Connected) {
//        //qDebug() << "QCoapConnection : sendRequest() - Connected";
//        d->_q_startToSendRequest();
//    }

    if (d->state == Bound || d->state == Connected) {
        qDebug() << "QCoapConnection : sendRequest() - Bound or Connected";
        // NOTE : QMetaObject::invokeMethod() ???
        d->_q_startToSendRequest();
    } else if (d->state == Unconnected) {
        qDebug() << "QCoapConnection : sendRequest() - Unconnected";
        connect(this, SIGNAL(bound()), this, SLOT(_q_startToSendRequest()));
        d->bindToHost();
    }
}

QByteArray QCoapConnection::readAll()
{
    Q_D(QCoapConnection);

    if (!d->udpSocket->isReadable())
        d->udpSocket->open(d->udpSocket->openMode() | QIODevice::ReadOnly);

    // Because for the tests we use a QBuffer that is not sequential
    if (!d->udpSocket->isSequential())
        d->udpSocket->seek(0);

    QByteArray reply;
    QUdpSocket* socket = static_cast<QUdpSocket*>(d->udpSocket);
    while (socket->hasPendingDatagrams())
    {
        QByteArray data = socket->receiveDatagram().data();
        qDebug() << "data : " << data;
        reply.append(data);
    }
    //QByteArray reply = d->udpSocket->readAll();
    if (!reply.isEmpty())
        d->lastReply = reply;

    return d->lastReply;
}

void QCoapConnectionPrivate::writeToSocket(const QByteArray& data)
{
    if (!udpSocket->isWritable())
        udpSocket->open(udpSocket->openMode() | QIODevice::WriteOnly);

    qDebug() << "QCoapConnection::writeToSocket()";
    //d->udpSocket->write(data);
    QUdpSocket* socket = static_cast<QUdpSocket*>(udpSocket);
    socket->writeDatagram(data, QHostAddress(host), port);
}

void QCoapConnectionPrivate::boundToHost()
{
    qDebug() << "QCoapConnectionPrivate::boundToHost()";
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::Bound)
        return;

     setState(QCoapConnection::Bound);
     emit q->bound();
}

void QCoapConnectionPrivate::_q_startToSendRequest()
{
    qDebug() << "QCoapConnectionPrivate::_q_startToSendRequest()";
    writeToSocket(currentPdu);
}

void QCoapConnectionPrivate::_q_connectedToHost()
{
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::Connected)
        return;

     setState(QCoapConnection::Connected);
     emit q->connected();
}

void QCoapConnectionPrivate::_q_disconnectedFromHost()
{
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::Unconnected)
        return;

     setState(QCoapConnection::Unconnected);
     emit q->disconnected();
}

void QCoapConnectionPrivate::_q_socketReadyRead()
{
    Q_Q(QCoapConnection);

    qDebug() << "QCoapConnectionPrivate::_q_socketReadyRead() - " << q->readAll();
    emit q->readyRead(q->readAll());
}

void QCoapConnectionPrivate::_q_socketError(QAbstractSocket::SocketError error)
{
    Q_Q(QCoapConnection);

    qDebug() << "Socket error" << error << udpSocket->errorString();
    emit q->error(error);
}

QString QCoapConnection::host() const
{
    return d_func()->host;
}

quint16 QCoapConnection::port() const
{
    return d_func()->port;
}

QIODevice* QCoapConnection::socket() const
{
    return d_func()->udpSocket;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state;
}

void QCoapConnection::setHost(const QString& host)
{
    Q_D(QCoapConnection);
    if (d->host == host)
        return;

    d->host = host;
}

void QCoapConnection::setPort(quint16 port)
{
    Q_D(QCoapConnection);
    if (d->port == port)
        return;

    d->port = port;
}

void QCoapConnectionPrivate::setSocket(QIODevice* device)
{
    if (udpSocket == device)
        return;

    udpSocket = device;
}

void QCoapConnectionPrivate::setState(QCoapConnection::QCoapConnectionState newState)
{
    if (state == newState)
        return;

    state = newState;
}

QT_END_NAMESPACE

#include "moc_qcoapconnection.cpp"
