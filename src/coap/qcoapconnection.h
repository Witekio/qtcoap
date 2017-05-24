#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QString>
#include <QtNetwork/QUdpSocket>

class QCoapConnection : public QObject
{
    Q_OBJECT

public:
    QCoapConnection(/*QObject* parent = nullptr*/);

    QString host();
    int port();
    QUdpSocket* socket();
};

#endif // QCOAPCONNECTION_H
