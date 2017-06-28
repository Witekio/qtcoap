#ifndef QCOAPDISCOVERYREPLY_P_H
#define QCOAPDISCOVERYREPLY_P_H

#include "qcoapdiscoveryreply.h"
#include "qcoapreply_p.h"

QT_BEGIN_NAMESPACE

class QCoapDiscoveryReplyPrivate : public QCoapReplyPrivate
{
public:
    QList<QCoapResource> resources;
};

QT_END_NAMESPACE

#endif // QCOAPDISCOVERYREPLY_P_H
