/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcoapconnection_p.h"
#include <QtNetwork/qnetworkdatagram.h>

QT_BEGIN_NAMESPACE

QCoapConnectionPrivate::QCoapConnectionPrivate() :
    state(QCoapConnection::Unconnected)
{
}

QCoapConnectionPrivate::~QCoapConnectionPrivate()
{
}

/*!
    \class QCoapConnection
    \brief The QCoapConnection class handles the transfer of a frame to a
    server.

    \reentrant

    The QCoapConnection class is used by the QCoapClient class to send
    requests to a server. It has a socket listening for UDP messages,
    that is used to send the coap frames.

    When a reply is available, the QCoapConnection object emits a
    \l{QCoapConnection::readyRead(const QByteArray&)}
    {readyRead(const QByteArray&)} signal

    \sa QCoapClient
*/

/*!
    Constructs a new QCoapConnection and sets \a parent as the parent object.
*/
QCoapConnection::QCoapConnection(QObject *parent) :
    QCoapConnection(*new QCoapConnectionPrivate, parent)
{
}

/*!
    \internal
    Constructs a new QCoapConnection with \a dd as the d_ptr.
    This constructor must be used when internally subclassing
    the QCoapConnection class.
*/
QCoapConnection::QCoapConnection(QCoapConnectionPrivate &dd, QObject *parent) :
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

    Binds the socket to a random port and return true if it succeeds.
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
void QCoapConnection::sendRequest(const QByteArray &request, const QString &host, quint16 port)
{
    Q_D(QCoapConnection);

    CoapFrame frame(request, host, port);
    d->framesToSend.enqueue(frame);

    if (d->state == Bound) {
        QMetaObject::invokeMethod(this, "_q_startToSendRequest", Qt::QueuedConnection);
    } else if (d->state == Unconnected) {
        connect(this, SIGNAL(bound()), this, SLOT(_q_startToSendRequest()), Qt::QueuedConnection);
        d->bindSocket();
    }
}

/*!
    \internal

    Writes the given \a data frame to the socket to the stored host and port.
*/
void QCoapConnectionPrivate::writeToSocket(const QByteArray &data, const QString &host, quint16 port)
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
    {readyRead(const QByteArray&)} signal for each received
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
QUdpSocket *QCoapConnection::socket() const
{
    Q_D(const QCoapConnection);
    return d->udpSocket;
}

/*!
    Returns the connection state.
*/
QCoapConnection::ConnectionState QCoapConnection::state() const
{
    Q_D(const QCoapConnection);
    return d->state;
}

/*!
    \internal

    Sets the socket.

    \sa socket()
*/
#if 0
void QCoapConnectionPrivate::setSocket(QUdpSocket *socket)
{
    udpSocket = socket;
}
#endif

/*!
    \internal

    Sets the connection state.

    \sa state()
*/
void QCoapConnectionPrivate::setState(QCoapConnection::ConnectionState newState)
{
    state = newState;
}

QT_END_NAMESPACE

#include "moc_qcoapconnection.cpp"
