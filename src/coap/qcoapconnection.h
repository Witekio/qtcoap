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
    Q_DISABLE_COPY(QCoapConnection)

public:
    enum QCoapConnectionState {
        UNCONNECTED,
        CONNECTED
    };

    enum QCoapRequestSendingState {
        WAITING,
        SENDING,
        COMPLETE,
    };

    explicit QCoapConnection(const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);
    QCoapConnection(QCoapConnectionPrivate& dd, const QString& host = "localhost", int port = 5683, QObject* parent = nullptr);

    void connectToHost();
    void sendRequest(const QByteArray& pduRequest);
    QByteArray readReply();

    QString host() const;
    int port() const;
    QIODevice* socket() const;
    QCoapConnectionState state() const;
    void setSocket(QIODevice* device);

signals:
    void connected();
    void readyRead();

private slots:
    void _q_connectedToHost();

protected:
    void writeToSocket(const QByteArray& data);

    Q_DECLARE_PRIVATE(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
