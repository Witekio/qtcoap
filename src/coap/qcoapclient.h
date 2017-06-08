#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QObject>
#include <QtCore/qglobal.h>

#include "qcoapreply.h"
#include "qcoaprequest.h"

QT_BEGIN_NAMESPACE

class QCoapClientPrivate;
class QCoapClient : public QObject
{
    Q_OBJECT

public:
    QCoapClient(QObject* parent = nullptr);

    QCoapReply* get(QCoapRequest* request,
                    QCoapRequest::QCoapMessageType type = QCoapRequest::NONCONFIRMABLE);

signals :
    void finished();

private slots:
    void requestFinished(QCoapRequest* request);

protected:
    int findRequestByToken(QByteArray token);
    bool containsToken(QByteArray token);
    bool containsMessageId(quint16 id);
    void sendRequest(QCoapRequest* request);
    void addRequest(QCoapRequest* request);

    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
