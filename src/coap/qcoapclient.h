#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QObject>
#include <QtCore/qglobal.h>

#include "qcoapresource.h"
#include "qcoapreply.h"
#include "qcoaprequest.h"

QT_BEGIN_NAMESPACE

class QCoapClientPrivate;
class QCoapClient : public QObject
{
    Q_OBJECT

public:
    QCoapClient(QObject* parent = nullptr);
    ~QCoapClient();

    QCoapReply* get(QCoapRequest* request);
    QCoapReply* put(QCoapRequest* request, const QByteArray& data = QByteArray());
    QCoapReply* post(QCoapRequest* request, const QByteArray& data = QByteArray());
    QCoapReply* deleteResource(QCoapRequest* request);
    QCoapReply* observe(QCoapRequest* request);
    //void cancelObserve(QCoapRequest* request);

    QList<QCoapResource*> discover(const QUrl& url, const QString& discoveryPath = "/.well-known/core");

signals :
    void finished();

protected:
    void sendRequest(QCoapRequest* request);
    void addRequest(QCoapRequest* request);

    Q_DECLARE_PRIVATE(QCoapClient)
    Q_PRIVATE_SLOT(d_func(), void _q_requestFinished(QCoapRequest* request))
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
