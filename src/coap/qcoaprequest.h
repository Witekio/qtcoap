#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>
#include <QTime>

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
        PUT,
        POST,
        DELETE,
        OTHER
    };

    QCoapRequest(const QUrl& url = QUrl(), QObject* parent = nullptr);
    //QCoapRequest(const QCoapRequest &other);

    QByteArray toPdu();
    void sendRequest();
    void readReply();
    quint16 generateMessageId();
    QByteArray generateToken();

    QUrl url() const;
    QCoapRequestOperation operation() const;
    QCoapReply* reply() const;
    QCoapConnection* connection() const;

    void setUrl(const QUrl& url);
    void setOperation(QCoapRequestOperation operation);

signals:
    void finished();

protected:
    void parseUri();

    Q_DECLARE_PRIVATE(QCoapRequest)
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
