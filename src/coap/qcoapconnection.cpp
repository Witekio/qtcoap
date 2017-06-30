#include "qcoapconnection.h"
#include "qcoapconnection_p.h"

#include <QNetworkDatagram>

QT_BEGIN_NAMESPACE

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host(""),
    port(0),
    udpSocket(nullptr),
    currentPdu(QByteArray()),
    state(QCoapConnection::UNCONNECTED)
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

void QCoapConnection::bindToHost()
{
    qDebug() << "QCoapConnection::bindToHost()";

    Q_D(QCoapConnection);

    QUdpSocket* socket = static_cast<QUdpSocket*>(d->udpSocket);
    if (socket->state() == QUdpSocket::ConnectedState)
        socket->disconnectFromHost();

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(readyRead()), this, SLOT(_q_socketReadyRead()));
    if (socket->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress)) {
        qDebug() << "QCoapConnection::bindToHost() - OK";
        d->boundToHost();
        qDebug() << socket->localPort();
    }
}

void QCoapConnection::sendRequest(const QByteArray& request)
{
    //qDebug() << "QCoapConnection::sendRequest()";
    Q_D(QCoapConnection);

    //setSendingState(QCoapConnection::WAITING);

    d->currentPdu = request;

    if (d->state == UNCONNECTED) {
        //qDebug() << "QCoapConnection : sendRequest() - UNCONNECTED";
        connect(this, SIGNAL(connected()), this, SLOT(_q_startToSendRequest()));
        connectToHost();
    } else if (d->state == CONNECTED) {
        //qDebug() << "QCoapConnection : sendRequest() - CONNECTED";
        d->_q_startToSendRequest();
    }

//    if (d->state == BOUND || d->state == CONNECTED) {
//        qDebug() << "QCoapConnection : sendRequest() - BOUND or CONNECTED";
//        // QMetaObject::invokeMethod() ???
//        d->_q_startToSendRequest();
//    } else if (d->state == UNCONNECTED) {
//        qDebug() << "QCoapConnection : sendRequest() - UNCONNECTED";
//        bindToHost();
//    }
}

QByteArray QCoapConnection::readAll()
{
    Q_D(QCoapConnection);

    if (!d->udpSocket->isReadable())
        d->udpSocket->open(d->udpSocket->openMode() | QIODevice::ReadOnly);

    // Because for the tests we use a QBuffer that is not sequential
    if (!d->udpSocket->isSequential())
        d->udpSocket->seek(0);

    QByteArray reply = d->udpSocket->readAll();
    if (!reply.isEmpty())
        d->lastReply = reply;

    return d->lastReply;
}

void QCoapConnection::writeToSocket(const QByteArray& data)
{
    Q_D(QCoapConnection);

    if (!d->udpSocket->isWritable())
        d->udpSocket->open(d->udpSocket->openMode() | QIODevice::WriteOnly);

    qDebug() << "QCoapConnection::writeToSocket()";
    d->udpSocket->write(data);
    //static_cast<QUdpSocket>(d->udpSocket).writeDatagram(data, QHostAddress(d->host), d->port);
}

// NOTE : Might be removed
void QCoapConnectionPrivate::boundToHost()
{
    qDebug() << "QCoapConnectionPrivate::boundToHost()";
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::BOUND)
        return;

     q->setState(QCoapConnection::BOUND);
     emit q->bound();
}

void QCoapConnectionPrivate::_q_startToSendRequest()
{
    //qDebug() << "QCoapConnectionPrivate::_q_startToSendRequest()";
    Q_Q(QCoapConnection);
    q->writeToSocket(currentPdu);
}

void QCoapConnectionPrivate::_q_connectedToHost()
{
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::CONNECTED)
        return;

     q->setState(QCoapConnection::CONNECTED);
     emit q->connected();
}

void QCoapConnectionPrivate::_q_disconnectedFromHost()
{
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::UNCONNECTED)
        return;

     q->setState(QCoapConnection::UNCONNECTED);
     emit q->disconnected();
}

void QCoapConnectionPrivate::_q_socketReadyRead()
{
    Q_Q(QCoapConnection);

    //qDebug() << "QCoapConnectionPrivate::_q_socketReadyRead() - " << q->readAll();
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

void QCoapConnection::setSocket(QIODevice* device)
{
    Q_D(QCoapConnection);
    if (d->udpSocket == device)
        return;

    d->udpSocket = device;
}

void QCoapConnection::setState(QCoapConnectionState state)
{
    Q_D(QCoapConnection);

    if (d->state == state)
        return;

    d->state = state;
}

QT_END_NAMESPACE

#include "moc_qcoapconnection.cpp"
