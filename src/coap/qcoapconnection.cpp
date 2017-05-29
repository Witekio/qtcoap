#include "qcoapconnection.h"
#include "qcoapconnection_p.h"

QCoapConnectionPrivate::QCoapConnectionPrivate()
{
}

QCoapConnection::QCoapConnection(const QString& host, int port, QObject* parent) :
    QObject(parent)
{
    Q_UNUSED(host);
    Q_UNUSED(port);
    //TODO
}

void QCoapConnection::connectToHost() {
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
    // TODO
    return QString("");
}

int QCoapConnection::port() const
{
    //TODO
    return -9999;
}

QUdpSocket* QCoapConnection::socket() const
{
    // TODO
    return nullptr;
}

QCoapConnection::QCoapConnectionState QCoapConnection::state() const
{
    // TODO
    return QCoapConnection::UNCONNECTED;
}
