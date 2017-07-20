#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include <QNetworkDatagram>

QT_BEGIN_NAMESPACE

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    udpSocket(nullptr),
    state(QCoapConnection::Unconnected)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
}

/*!
    \class QCoapConnection
    \brief The QCoapConnection class handle the transfer of a frame to a
    server.

    \reentrant

    The QCoapConnection class is used by the QCoapClient class to send the
    requests to a server. It has a socket listening for udp messages and
    that is used to send the coap frames.

    When a reply is available, the QCoapConnection object emits a
    \l{QCoapConnection::readyRead(const QByteArray&)}
    {readyRead(const QByteArray&)} signal


    \sa QCoapClient
*/

/*!
    Constructs a new QCoapConnection and sets \a parent as the parent object.
*/
QCoapConnection::QCoapConnection(QObject* parent) :
    QCoapConnection(*new QCoapConnectionPrivate, parent)
{
}

/*!
    \internal
    Constructs a new QCoapConnection with \a dd as the d_ptr.
    This constructor must be used when subclassing internally
    the QCoapConnection class.
*/
QCoapConnection::QCoapConnection(QCoapConnectionPrivate& dd, QObject* parent) :
    QObject(dd, parent)
{
    Q_D(QCoapConnection);
    d->udpSocket = new QUdpSocket(this);
}

/*!
    Destroys the QCoapConnection and frees the socket.
*/
QCoapConnection::~QCoapConnection()
{
    Q_D(QCoapConnection);
    delete d->udpSocket;
}

/*!
    \internal

    Binds the socket to a random port and return true if it bounds with
    Ssuccess.
*/
bool QCoapConnectionPrivate::bind()
{
    return udpSocket->bind(QHostAddress::Any, 0, QAbstractSocket::ShareAddress);
}

/*!
    \internal

    Binds the socket and call the socketBound() slot.
*/
void QCoapConnectionPrivate::bindSocket()
{
    Q_Q(QCoapConnection);

    if (udpSocket->state() == QUdpSocket::ConnectedState)
        udpSocket->disconnectFromHost();

    q->connect(udpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(_q_socketError(QAbstractSocket::SocketError)));
    q->connect(udpSocket, SIGNAL(readyRead()), q, SLOT(_q_socketReadyRead()));

    if (bind())
        _q_socketBound();
}

/*!
    Binds the socket if it is not already done and sends the given
    \a request frame to the given \a host and \a port.
*/
void QCoapConnection::sendRequest(const QByteArray& request, const QString& host, quint16 port)
{
    Q_D(QCoapConnection);

    CoapFrame frame = {request, host, port};
    d->framesToSend.enqueue(frame);

    if (d->state == Bound) {
        QMetaObject::invokeMethod(this, "_q_startToSendRequest");
    } else if (d->state == Unconnected) {
        connect(this, SIGNAL(bound()), this, SLOT(_q_startToSendRequest()));
        d->bindSocket();
    }
}

/*!
    \internal

    Writes the given \a data frame to the socket to the stored host and port.
*/
void QCoapConnectionPrivate::writeToSocket(const QByteArray& data, const QString& host, quint16 port)
{
    if (!udpSocket->isWritable())
        udpSocket->open(udpSocket->openMode() | QIODevice::WriteOnly);

    udpSocket->writeDatagram(data, QHostAddress(host), port);
}

/*!
    \internal

    This slot changes the connection state to "Bound" and emits the
    \l{QCoapConnection::bound()}{bound()} signal.
*/
void QCoapConnectionPrivate::_q_socketBound()
{
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
    CoapFrame frame = framesToSend.dequeue();
    writeToSocket(frame.currentPdu, frame.host, frame.port);
}

/*!
    \internal

    This slot reads all data stored in the socket and emits
    \l{QCoapConnection::readyRead(const QByteArray&)}
    {readyRead(const QByteArray&)} signal for all received
    datagram.
*/
void QCoapConnectionPrivate::_q_socketReadyRead()
{
    Q_Q(QCoapConnection);

    if (!udpSocket->isReadable())
        udpSocket->open(udpSocket->openMode() | QIODevice::ReadOnly);

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray data = udpSocket->receiveDatagram().data();
        emit q->readyRead(data);
    }
}

/*!
    \internal

    This slot emits the \l{QCoapConnection::error(QAbstractSocket::SocketError)}
    {error(QAbstractSocket::SocketError)} signal.
*/
void QCoapConnectionPrivate::_q_socketError(QAbstractSocket::SocketError error)
{
    Q_Q(QCoapConnection);

    qDebug() << "CoAP UDP socket error " << error << udpSocket->errorString();
    emit q->error(error);
}

/*!
    Returns the socket.
*/
QUdpSocket* QCoapConnection::socket() const
{
    return d_func()->udpSocket;
}

/*!
    Returns the connection state.
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
void QCoapConnectionPrivate::setSocket(QUdpSocket* socket)
{
    udpSocket = socket;
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
