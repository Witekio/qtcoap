#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QString>
#include <QtNetwork/QUdpSocket>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate;
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

private:
    Q_DECLARE_PRIVATE(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
