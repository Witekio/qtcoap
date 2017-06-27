#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QString>
#include <QUdpSocket>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate;
class QCoapConnection : public QObject
{
    Q_OBJECT

public:
    enum QCoapConnectionState {
        UNCONNECTED,
        CONNECTED,
        BOUND
    };

    enum QCoapConnectionSendingState {
        WAITING,
        SENDING,
        COMPLETE,
    };

    explicit QCoapConnection(const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);
    QCoapConnection(QCoapConnectionPrivate& dd, const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);
    ~QCoapConnection();

    void connectToHost();
    void sendRequest(const QByteArray& pduRequest);
    QByteArray readAll();

    QString host() const;
    int port() const;
    QIODevice* socket() const;
    QCoapConnectionState state() const;
    void setHost(const QString& host);
    void setPort(int port);

signals:
    void connected();
    void disconnected();
    void bound();
    void readyRead(const QByteArray& frame);

private slots:
    void onSocketError(QAbstractSocket::SocketError error);

protected:
    void bindToHost();
    void writeToSocket(const QByteArray& data);
    void setSocket(QIODevice* device);
    void setState(QCoapConnectionState state);
    void setSendingState(QCoapConnectionSendingState sendingState);

    Q_DECLARE_PRIVATE(QCoapConnection)
    Q_PRIVATE_SLOT(d_func(), void _q_connectedToHost())
    Q_PRIVATE_SLOT(d_func(), void _q_disconnectedFromHost())
    Q_PRIVATE_SLOT(d_func(), void _q_socketReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_startToSendRequest())
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
