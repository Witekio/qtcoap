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
        Bound
    };

    explicit QCoapConnection(QObject* parent = nullptr);
    ~QCoapConnection();

    void sendRequest(const QByteArray& request, const QString& host, quint16 port);
    QByteArray readAll();

    QString host() const;
    quint16 port() const;
    QIODevice* socket() const;
    QCoapConnectionState state() const;

signals:
    void bound();
    void error(QAbstractSocket::SocketError);
    void readyRead(const QByteArray& frame);

protected:
    QCoapConnection(QCoapConnectionPrivate& dd, QObject* parent = nullptr);

    Q_DECLARE_PRIVATE(QCoapConnection)
    Q_PRIVATE_SLOT(d_func(), void _q_socketReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_socketBound())
    Q_PRIVATE_SLOT(d_func(), void _q_startToSendRequest())
    Q_PRIVATE_SLOT(d_func(), void _q_socketError(QAbstractSocket::SocketError))
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
