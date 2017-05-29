#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>

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
    qint16 generateMessageId();
    qint64 generateToken();

    QUrl url() const;
    QCoapRequestOperation operation() const;
    QCoapReply* reply();

    void setUrl(const QUrl& url);
    void setOperation(QCoapRequestOperation operation);

signals:
    void finished();

protected:
    //QCoapConnection* connection_p;
    //QCoapReply* reply_p;

    Q_DECLARE_PRIVATE(QCoapRequest)
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
