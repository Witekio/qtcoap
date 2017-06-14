#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include <QNetworkDatagram>

QT_BEGIN_NAMESPACE

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    host(""),
    port(0),
    udpSocket(new QUdpSocket()),
    currentPdu(QByteArray()),
    state(QCoapConnection::UNCONNECTED),
    sendingState(QCoapConnection::WAITING)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
    if (udpSocket) {
        if (udpSocket->isOpen())
            udpSocket->close();
        delete udpSocket;
    }
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

    QUdpSocket * socket = qobject_cast<QUdpSocket *>(d->udpSocket);

    connect(socket, SIGNAL(connected()), this, SLOT(_q_connectedToHost()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(_q_socketReadyRead()));

    socket->connectToHost(d->host, d->port);
}

#include <QThread>

void QCoapConnection::sendRequest(const QByteArray& request)
{
    Q_D(QCoapConnection);

    setSendingState(QCoapConnection::WAITING);

    d->currentPdu = request;
    if (d->state == UNCONNECTED) {
        qDebug() << "QCoapConnection : sendRequest() - UNCONNECTED";
        connect(this, SIGNAL(connected()), this, SLOT(_q_startToSendRequest()));
        connectToHost();
    } else if (d->state == CONNECTED) {
        qDebug() << "QCoapConnection : sendRequest() - CONNECTED";
        //QThread::msleep(100);
        d->_q_startToSendRequest();
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

void QCoapConnectionPrivate::_q_startToSendRequest()
{
    Q_Q(QCoapConnection);

    if (sendingState == QCoapConnection::WAITING) {
        q->setSendingState(QCoapConnection::SENDING);
        q->writeToSocket(currentPdu);
    }
}

void QCoapConnectionPrivate::_q_connectedToHost()
{
    Q_Q(QCoapConnection);

    if (state == QCoapConnection::CONNECTED)
        return;

     q->setState(QCoapConnection::CONNECTED);
     emit q->connected();
}

void QCoapConnectionPrivate::_q_socketReadyRead()
{
    Q_Q(QCoapConnection);

    if (sendingState == QCoapConnection::COMPLETE)
        return;

    q->setSendingState(QCoapConnection::COMPLETE);
    emit q->readyRead();
}

QString QCoapConnection::host() const
{
    return d_func()->host;
}

int QCoapConnection::port() const
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

void QCoapConnection::setPort(int port)
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

void QCoapConnection::setSendingState(QCoapConnectionSendingState sendingState)
{
    Q_D(QCoapConnection);

    if (d->sendingState == sendingState)
        return;

    d->sendingState = sendingState;
}

QT_END_NAMESPACE

#include "moc_qcoapconnection.cpp"
