#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>

#include "qcoapmessage.h"
#include "qcoapconnection.h"
#include "qcoapreply.h"

class QCoapRequest : public QObject, public QCoapMessage
{
    Q_OBJECT
public:
    enum QCoapRequestOperation {
        GET,
        PUT,
        POST,
        DELETE,
        OTHER
    };

    QCoapRequest(const QUrl& url = QUrl(), QObject* parent = nullptr);

    QByteArray toPdu();
    void sendRequest();
    void readReply();

    QUrl url() const;
    QCoapRequestOperation operation() const;
    QByteArray rawReply();

    void setUrl(const QUrl& url);
    void setOperation(QCoapRequestOperation operation);

signals:
    void finished();

protected:
    QCoapConnection* connection_p;
    QCoapReply* reply_p;
};

#endif // QCOAPREQUEST_H
