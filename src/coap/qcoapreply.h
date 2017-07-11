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

class QCoapReplyPrivate;
class QCoapReply : public QIODevice
{
    Q_OBJECT
public:
    enum QCoapNetworkError {
        NoCoapError,
        HostNotFoundCoapError,
        BadRequestCoapError,
        TimeOutCoapError,
        UnknownCoapError
    };

    QCoapReply(QObject* parent = Q_NULLPTR);
    ~QCoapReply();

    QCoapStatusCode statusCode() const;
    QCoapMessage message() const;
    QCoapRequest request() const;
    QUrl url() const;
    QCoapOperation operation() const;
    QCoapNetworkError errorReceived() const;
    bool isRunning() const;
    bool isFinished() const;
    bool isAborted() const;
    void setRequest(const QCoapRequest& request);

signals:
    void finished();
    void notified(const QByteArray&);
    void error(QCoapReply::QCoapNetworkError);
    void aborted(QCoapReply*);

private slots:
    void connectionError(QAbstractSocket::SocketError);
    void replyError(QCoapStatusCode);

public slots:
    void abortRequest();

protected:
    friend class QCoapProtocol;
    friend class QCoapProtocolPrivate;

    QCoapReply(QCoapReplyPrivate &dd, QObject* parent = Q_NULLPTR);

    void setIsRunning(bool isRunning);
    void setError(QCoapNetworkError error);
    virtual void updateFromInternalReply(const QCoapInternalReply& internalReply); // remove public (friend ?)
    qint64 readData(char* data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char* data, qint64 maxSize) Q_DECL_OVERRIDE;

    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCoapReply::QCoapNetworkError)

#endif // QCOAPREPLY_H
