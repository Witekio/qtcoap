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

    QCoapReply* get(const QCoapRequest& request);

signals :
    void finished();

private:
    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
