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
        Unconnected,
        Connected,
        Bound
    };

    explicit QCoapConnection(const QString& host = "localhost", quint16 port = 5683, QObject* parent = nullptr);
    ~QCoapConnection();

    void connectToHost();
    void sendRequest(const QByteArray& request, const QString& host, quint16 port);
    QByteArray readAll();

    QString host() const;
    quint16 port() const;
    QIODevice* socket() const;
    QCoapConnectionState state() const;
    void setHost(const QString& host);
    void setPort(quint16 port);

signals:
    void connected();
    void disconnected();
    void bound();
    void error(QAbstractSocket::SocketError);
    void readyRead(const QByteArray& frame);

protected:
    QCoapConnection(QCoapConnectionPrivate& dd, const QString& host = "localhost", quint16 port = 5683, QObject* parent = nullptr);

    void bindToHost();
    void writeToSocket(const QByteArray& data);
    void setSocket(QIODevice* device);
    void setState(QCoapConnectionState state);

    Q_DECLARE_PRIVATE(QCoapConnection)
    Q_PRIVATE_SLOT(d_func(), void _q_connectedToHost())
    Q_PRIVATE_SLOT(d_func(), void _q_disconnectedFromHost())
    Q_PRIVATE_SLOT(d_func(), void _q_socketReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_startToSendRequest())
    Q_PRIVATE_SLOT(d_func(), void _q_socketError(QAbstractSocket::SocketError))
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
