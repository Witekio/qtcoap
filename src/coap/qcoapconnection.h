#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QString>
#include <QtNetwork/QUdpSocket>
#include <qcoaprequest.h>

class QCoapConnection : public QObject
{
    Q_OBJECT

public:
    QCoapConnection(const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);

    void sendRequest(const QCoapRequest& request);
    QByteArray readReply();

    QString host() const;
    int port() const;
    QUdpSocket* socket() const;

signals:
    void readyRead();

protected:
    QUdpSocket* udpSocket;
};

#endif // QCOAPCONNECTION_H
