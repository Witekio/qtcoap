#ifndef QCOAPCONNECTION_P_H
#define QCOAPCONNECTION_P_H

#include "qcoapconnection.h"
#include <private/qobject_p.h>
#include <QUdpSocket>
#include <QQueue>

QT_BEGIN_NAMESPACE

struct CoapFrame {
    QByteArray currentPdu;
    QString host;
    quint16 port;
};

class QCoapConnectionPrivate : public QObjectPrivate
{
public:
    QCoapConnectionPrivate();
    ~QCoapConnectionPrivate();

    QUdpSocket* udpSocket;
    QCoapConnection::QCoapConnectionState state;
    QQueue<CoapFrame> framesToSend;

    virtual bool bind();

    void bindSocket();
    void writeToSocket(const QByteArray& data, const QString& host, quint16 port);
    void setSocket(QUdpSocket* socket);
    void setState(QCoapConnection::QCoapConnectionState newState);

    void _q_socketBound();
    void _q_socketReadyRead();
    void _q_startToSendRequest();
    void _q_socketError(QAbstractSocket::SocketError);

    Q_DECLARE_PUBLIC(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_P_H
