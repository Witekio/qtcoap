#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include "qcoapmessage.h"
#include "qcoapinternalreply_p.h"
#include "qcoaprequest.h"
#include "qcoapglobal.h"
#include <QByteArray>
#include <QIODevice>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE

//class QCoapRequest;
class QCoapReplyPrivate;
class QCoapReply : public QIODevice
{
    Q_OBJECT
public:
    enum QCoapNetworkError {
        NoCoapError,
        HostNotFoundCoapError,
        BadRequestCoapError,
        UnknownCoapError
    };

    QCoapReply(QObject* parent = Q_NULLPTR);
    ~QCoapReply();

    QCoapStatusCode statusCode() const;
    QCoapMessage message() const;
    QCoapRequest request() const;
    QUrl url() const;
    QCoapOperation operation() const;
    bool isRunning() const;
    bool isFinished() const;
    void setRequest(const QCoapRequest& request);

signals:
    void finished();
    void notified(const QByteArray& data);
    // TODO : handle error(QCoapError) signal
    void error(QCoapNetworkError);
    void abortRequest(QCoapReply*);

private slots:
    void connectionError(QAbstractSocket::SocketError);
    void replyError(QCoapStatusCode);

protected:
    friend class QCoapProtocol;
    friend class QCoapProtocolPrivate;

    QCoapReply(QCoapReplyPrivate &dd, QObject* parent = Q_NULLPTR);

    void setIsRunning(bool isRunning);
    virtual void updateFromInternalReply(const QCoapInternalReply& internalReply); // remove public (friend ?)
    qint64 readData(char* data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char* data, qint64 maxSize) Q_DECL_OVERRIDE;

    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_H
