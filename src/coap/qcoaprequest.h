#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>
#include <QTime>
#include <QThread>
#include <QSharedDataPointer>

#include "qcoapmessage.h"
#include "qcoapconnection.h"
#include "qcoapreply.h"
#include "qcoapprotocol.h"

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate;
class QCoapRequest : public QCoapMessage
{
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
        WAITING,
        SENT,
        REPLIED,
        REPLYCOMPLETE
    };

    QCoapRequest(const QUrl& url = QUrl(),
                 QCoapMessageType type = NONCONFIRMABLE);
    QCoapRequest(const QCoapRequest &other);
    ~QCoapRequest();

    QCoapRequest& operator=(const QCoapRequest& other);

    QByteArray toPdu();
    void sendRequest();
    quint16 generateMessageId();
    QByteArray generateToken();
    void addOptionToAskBlock(uint blockNumber);
    void initializeAttributesFrom(QCoapRequest* request);

    QUrl url() const;
    QCoapRequestOperation operation() const;
    QCoapReply* reply() const;
    QCoapConnection* connection() const;
    bool observe() const;
    void setUrl(const QUrl& url);
    void setOperation(QCoapRequestOperation operation);
    void setObserve(bool observe);

// TODO : remove signals and slots and make private internal class
//signals:
    // void notified(const QByteArray& replyData); // NOTE : will certainly be removed
    //void finished(QCoapRequest* request);
    // void replied(); // NOTE : will certainly be removed

protected:
    void parseUri();
    void setReply(QCoapReply* reply);
    void setConnection(QCoapConnection* connection);
    void setProtocol(QCoapProtocol* protocol);
    void setState(QCoapRequestState state);
    void setRequestForAck(quint16 messageId, const QByteArray& payload = QByteArray());
    void setRequestForReset(quint16 messageId);
    void readReply(); // TODO : remove readReply and find another way to test _q_readReply

    QSharedDataPointer<QCoapRequestPrivate> d_ptr;

    /*Q_DECLARE_PRIVATE(QCoapRequest)
    Q_PRIVATE_SLOT(d_func(), void _q_startToSend())
    //Q_PRIVATE_SLOT(d_func(), void _q_readReply())
    //Q_PRIVATE_SLOT(d_func(), void _q_getNextBlock(uint blockAsked))
    //Q_PRIVATE_SLOT(d_func(), void _q_sendAck(quint16 messageId))*/
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
