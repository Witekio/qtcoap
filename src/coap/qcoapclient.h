#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include "qcoapglobal.h"
#include "qcoapresource.h"
#include "qcoapreply.h"
#include "qcoaprequest.h"
#include "qcoapprotocol.h"
#include "qcoapdiscoveryreply.h"
#include <QObject>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapClientPrivate;
class QCoapClient : public QObject
{
    Q_OBJECT
public:
    QCoapClient(QObject* parent = nullptr);
    ~QCoapClient();

    QCoapReply* get(const QCoapRequest& request);
    QCoapReply* put(const QCoapRequest& request, const QByteArray& data = QByteArray());
    QCoapReply* put(const QCoapRequest& request, QIODevice* device);
    QCoapReply* post(const QCoapRequest& request, const QByteArray& data = QByteArray());
    QCoapReply* post(const QCoapRequest& request, QIODevice* device);
    QCoapReply* deleteResource(const QCoapRequest& request);
    QCoapReply* observe(const QCoapRequest& request);
    void cancelObserve(const QCoapRequest& request);
    void cancelObserve(QCoapReply* notifiedReply);

    QCoapDiscoveryReply* discover(const QUrl& url,
                                  const QString& discoveryPath = "/.well-known/core");

    QCoapProtocol* protocol() const;
    QCoapConnection* connection() const;
    void setBlockSize(quint16 blockSize);
    void enableMulticastLoopbackOption();
    void setMulticastTtlOption(int ttlValue);
    void setProtocol(QCoapProtocol* protocol);

signals:
    void finished(QCoapReply*);

protected:
    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
