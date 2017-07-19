#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include "qcoapglobal.h"
#include "qcoapmessage.h"
#include "qcoapconnection.h"
#include "qcoapinternalrequest_p.h"
#include <QObject>
#include <QUrl>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate;
class QCoapRequest : public QCoapMessage
{
public:
    QCoapRequest(const QUrl& url = QUrl(),
                 QCoapMessageType type = NonConfirmableCoapMessage,
                 const QUrl& proxyUrl = QUrl());
    QCoapRequest(const QCoapRequest &other);
    ~QCoapRequest() {}

    QCoapRequest& operator=(const QCoapRequest& other);
    bool operator<(const QCoapRequest& other) const;

    QUrl url() const;
    QUrl proxyUrl() const;
    QCoapOperation operation() const;
    bool observe() const;
    void setUrl(const QUrl& url);
    void setProxyUrl(const QUrl& proxyUrl);
    void setOperation(QCoapOperation operation);
    void setObserve(bool observe);
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
