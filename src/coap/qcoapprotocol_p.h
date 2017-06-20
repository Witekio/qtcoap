#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include "qcoapprotocol.h"
#include "private/qobject_p.h"
#include "qcoapinternalrequest.h"
#include "qcoapinternalreply.h"

QT_BEGIN_NAMESPACE

class QCoapProtocolPrivate : public QObjectPrivate
{
public:
    QCoapInternalReply internalReply;
    QCoapInternalRequest internalRequest;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
