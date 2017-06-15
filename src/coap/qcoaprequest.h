#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>
#include <QTime>
#include <QThread>

#include "qcoapmessage.h"
#include "qcoapconnection.h"
#include "qcoapreply.h"

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate;
class QCoapRequest : public QCoapMessage
{
    Q_OBJECT
public:
    enum QCoapRequestOperation {
        EMPTY,
        GET,
        POST,
        PUT,
        DELETE,
        OTHER
    };

    enum QCoapRequestState {
        CREATED,
        SENT,
        REPLIED
    };

    QCoapRequest(const QUrl& url = QUrl(),
                 QCoapMessageType type = NONCONFIRMABLE,
                 QObject* parent = nullptr);
    //QCoapRequest(const QCoapRequest &other);

    QByteArray toPdu();
    void sendRequest();
    quint16 generateMessageId();
    QByteArray generateToken();

    QUrl url() const;
    QCoapRequestOperation operation() const;
    QCoapReply* reply() const;
    QCoapConnection* connection() const;
    bool observe() const;
    void setUrl(const QUrl& url);
    void setOperation(QCoapRequestOperation operation);
    void setObserve(bool observe);

signals:
    void notified(const QByteArray& replyData);
    void finished(QCoapRequest* request);
    void replied();

protected:
    void parseUri();
    void setReply(QCoapReply* reply);
    void setConnection(QCoapConnection* connection);
    void setState(QCoapRequestState state);
    void sendAck(quint16 messageId, const QByteArray& payload = QByteArray());
    void sendReset(quint16 messageId);
    void readReply(); // TODO : remove readReply and find another way to test _q_readReply

    Q_DECLARE_PRIVATE(QCoapRequest)
    Q_PRIVATE_SLOT(d_func(), void _q_startToSend())
    Q_PRIVATE_SLOT(d_func(), void _q_readReply())
    Q_PRIVATE_SLOT(d_func(), void _q_getNextBlock(uint blockAsked))
    Q_PRIVATE_SLOT(d_func(), void _q_sendAck(quint16 messageId))
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
