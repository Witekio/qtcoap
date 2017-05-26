#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QString>
#include <QtNetwork/QUdpSocket>

class QCoapConnection : public QObject
{
    Q_OBJECT

public:
    enum QCoapConnectionState {
        UNCONNECTED,
        CONNECTED
    };

    QCoapConnection(const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);

    void connectToHost();
    void sendRequest(const QByteArray& pduRequest);
    QByteArray readReply();

    QString host() const;
    int port() const;
    QUdpSocket* socket() const;
    QCoapConnectionState state() const;

signals:
    void readyRead();

protected:
    QIODevice* udpSocket;
};

#endif // QCOAPCONNECTION_H
