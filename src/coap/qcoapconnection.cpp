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

/*!
    \class QCoapConnection
    \brief The QCoapConnection class handle the transfer of a frame to a server.

    \reentrant

    The QCoapConnection is used by the QCoapClient class to send the requests to
    a server. It has a socket listening for udp messages and that is used to send
    the coap frames.

    \sa QCoapClient
*/

/*!
    Constructs a new QCoapConnection and sets \a parent as the parent object.
*/
// NOTE : host and port will be removed from the constructor
QCoapConnection::QCoapConnection(const QString& host, quint16 port, QObject* parent) :
    QCoapConnection(*new QCoapConnectionPrivate, host, port, parent)
{
}

/*!
    \internal
*/
// NOTE : host and port will be removed from the constructor
QCoapConnection::QCoapConnection(QCoapConnectionPrivate& dd, const QString& host, quint16 port, QObject* parent) :
    QObject(dd, parent)
{
    Q_D(QCoapConnection);

    d->host = host;
    d->port = port;

    // Make the socket a child of the connection to move it in the same thread
    d->udpSocket = new QUdpSocket(this);
}

/*!
    Destroy the QCoapConnection and frees the socket.
*/
QCoapConnection::~QCoapConnection()
{
    Q_D(QCoapConnection);
    delete d->udpSocket;
}

/*!
    \internal

    Binds the socket to a random port and return true if it bounds with success.
*/
bool QCoapConnectionPrivate::bind()
{
    QUdpSocket* socket = static_cast<QUdpSocket*>(udpSocket);
    return socket->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress);
}

/*!
    \internal

    Binds the socket and call the socketBound() slot.
*/
void QCoapConnectionPrivate::bindSocket()
{
    qDebug() << "QCoapConnection::bindToHost()";

    Q_Q(QCoapConnection);

    QUdpSocket* socket = static_cast<QUdpSocket*>(udpSocket);
    if (socket->state() == QUdpSocket::ConnectedState)
        socket->disconnectFromHost();

    q->connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), q, SLOT(_q_socketError(QAbstractSocket::SocketError)));
    q->connect(socket, SIGNAL(readyRead()), q, SLOT(_q_socketReadyRead()));
    if (bind()) {
        qDebug() << "QCoapConnection::bindToHost() - OK";
        qDebug() << "socket port : " << socket->localPort();
        _q_socketBound();
    }
}

/*!
    It binds the socket if it is not already done and sends the given
    \a request frame to the given \a host and \a port.
*/
void QCoapConnection::sendRequest(const QByteArray& request, const QString& host, quint16 port)
{
    //qDebug() << "QCoapConnection::sendRequest()";
    Q_D(QCoapConnection);

    d->currentPdu = request;
    d->host = host;
    d->port = port;

    if (d->state == Bound) {
        qDebug() << "QCoapConnection : sendRequest() - Bound or Connected";
        // NOTE : QMetaObject::invokeMethod() ???
        d->_q_startToSendRequest();
    } else if (d->state == Unconnected) {
        qDebug() << "QCoapConnection : sendRequest() - Unconnected";
        connect(this, SIGNAL(bound()), this, SLOT(_q_startToSendRequest()));
        d->bindSocket();
    }
}

/*!
    Reads all data stored in the socket.
*/
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

/*!
    \internal

    Writes the given \a data to the socket to the stored host and port.
*/
void QCoapConnectionPrivate::writeToSocket(const QByteArray& data)
{
    if (!udpSocket->isWritable())
        udpSocket->open(udpSocket->openMode() | QIODevice::WriteOnly);

    qDebug() << "QCoapConnection::writeToSocket()";
    //d->udpSocket->write(data);
    QUdpSocket* socket = static_cast<QUdpSocket*>(udpSocket);
    socket->writeDatagram(data, QHostAddress(host), port);
}

/*!
    \internal

    This slot changes the connection state to "Bound" and emit the
    bound signal.
*/
void QCoapConnectionPrivate::_q_socketBound()
{
    qDebug() << "QCoapConnectionPrivate::_q_socketBound()";
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::Bound)
        return;

     setState(QCoapConnection::Bound);
     emit q->bound();
}

/*!
    \internal

    This slot writes the current stored frame to the socket.
*/
void QCoapConnectionPrivate::_q_startToSendRequest()
{
    qDebug() << "QCoapConnectionPrivate::_q_startToSendRequest()";
    writeToSocket(currentPdu);
}

/*!
    \internal

    This slot emits the readyRead signal.
*/
void QCoapConnectionPrivate::_q_socketReadyRead()
{
    Q_Q(QCoapConnection);

    qDebug() << "QCoapConnectionPrivate::_q_socketReadyRead() - " << q->readAll();
    emit q->readyRead(q->readAll());
}

/*!
    \internal

    This slot emits the error signal.
*/
void QCoapConnectionPrivate::_q_socketError(QAbstractSocket::SocketError error)
{
    Q_Q(QCoapConnection);

    qDebug() << "Socket error" << error << udpSocket->errorString();
    emit q->error(error);
}

/*!
    Returns the host stored.
*/
QString QCoapConnection::host() const
{
    return d_func()->host;
}

/*!
    Returns the host stored.
*/
quint16 QCoapConnection::port() const
{
    return d_func()->port;
}

/*!
    Returns the socket.

    \sa setSocket()
*/
QIODevice* QCoapConnection::socket() const
{
    return d_func()->udpSocket;
}

/*!
    Returns the connection state.

    \sa setState()
*/
QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    return d_func()->state;
}

/*!
    \internal

    Sets the socket.

    \sa socket()
*/
void QCoapConnectionPrivate::setSocket(QIODevice* device)
{
    udpSocket = device;
}

/*!
    \internal

    Sets the connection state.

    \sa state()
*/
void QCoapConnectionPrivate::setState(QCoapConnection::QCoapConnectionState newState)
{
    state = newState;
}

QT_END_NAMESPACE

#include "moc_qcoapconnection.cpp"
