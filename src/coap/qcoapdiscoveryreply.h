#ifndef QCOAPDISCOVERYREPLY_H
#define QCOAPDISCOVERYREPLY_H

#include "qcoapreply.h"
#include "qcoapresource.h"
#include "qcoapprotocol.h"
#include <QList>

QT_BEGIN_NAMESPACE

class QCoapDiscoveryReplyPrivate;
class QCoapDiscoveryReply : public QCoapReply
{
public:
    QCoapDiscoveryReply(QObject *parent = nullptr);

    QList<QCoapResource> resourceList() const;

private:
    void updateFromInternalReply(const QCoapInternalReply& internalReply) Q_DECL_OVERRIDE;
    Q_DECLARE_PRIVATE(QCoapDiscoveryReply)

};

QT_END_NAMESPACE

#endif // QCOAPDISCOVERYREPLY_H
