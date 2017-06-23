#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include "qcoapprotocol.h"
#include "private/qobject_p.h"
#include <QList>

QT_BEGIN_NAMESPACE

typedef QMap<QCoapInternalRequest, QList<QCoapInternalReply>> InternalMessageMap;

class QCoapProtocolPrivate : public QObjectPrivate
{
public:
    QCoapProtocolPrivate();

    InternalMessageMap internalReplies;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
