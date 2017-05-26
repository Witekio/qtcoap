#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QObject>
#include "qcoapreply.h"
#include "qcoaprequest.h"

class QCoapClient : public QObject
{
    Q_OBJECT

public:
    QCoapClient(QObject* parent = nullptr);

    QCoapReply* get(const QCoapRequest& request);

signals :
    void finished();
};

#endif // QCOAPCLIENT_H
