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

    void boundToHost();

    void _q_connectedToHost();
    void _q_disconnectedFromHost();
    void _q_socketReadyRead();
    void _q_startToSendRequest();

    Q_DECLARE_PUBLIC(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_P_H
