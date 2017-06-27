#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>
#include <QTime>
#include <QThread>
#include <QSharedDataPointer>

#include "qcoapglobal.h"
#include "qcoapmessage.h"
#include "qcoapconnection.h"
//#include "qcoapprotocol.h"
#include "qcoapinternalrequest_p.h"
//#include "qcoapreply.h"

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate;
class QCoapRequest : public QCoapMessage
{
public:
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
    bool operator<(const QCoapRequest& other) const;

    QUrl url() const;
    QCoapOperation operation() const;
    //QCoapReply* reply() const;
    QCoapConnection* connection() const;
    bool observe() const;
    void setUrl(const QUrl& url);
    void setOperation(QCoapOperation operation);
    void setObserve(bool observe);

protected:
    void parseUri();
    //void setReply(QCoapReply* reply);
    void setConnection(QCoapConnection* connection);
    //void setProtocol(QCoapProtocol* protocol);
    void setState(QCoapRequestState state);
    //void setRequestForAck(quint16 messageId, const QByteArray& payload = QByteArray());
    //void setRequestForReset(quint16 messageId);
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
