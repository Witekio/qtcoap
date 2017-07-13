#ifndef QCOAPCONNECTION_P_H
#define QCOAPCONNECTION_P_H

#include "qcoapconnection.h"
#include <private/qobject_p.h>
#include <QIODevice>

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate : public QObjectPrivate
{
public:
    QCoapConnectionPrivate();
    ~QCoapConnectionPrivate();

    QString host;
    quint16 port;
    QIODevice* udpSocket;
    QByteArray currentPdu;
    QCoapConnection::QCoapConnectionState state;
    QByteArray lastReply; // Temporary store the last reply

    virtual bool bind();

    void bindSocket();
    void writeToSocket(const QByteArray& data);
    void setSocket(QIODevice* device);
    void setState(QCoapConnection::QCoapConnectionState newState);

    void _q_socketBound();
    void _q_socketReadyRead();
    void _q_startToSendRequest();
    void _q_socketError(QAbstractSocket::SocketError);

    Q_DECLARE_PUBLIC(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_P_H
