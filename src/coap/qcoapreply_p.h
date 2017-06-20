#ifndef QCOAPREPLY_P_H
#define QCOAPREPLY_P_H

#include "qcoapreply.h"
#include "qcoapmessage_p.h"
#include "private/qiodevice_p.h"

QT_BEGIN_NAMESPACE

class QCoapReplyPrivate : public QCoapMessagePrivate, public QIODevicePrivate
{
public:
    QCoapReplyPrivate();

    QCoapReply::QCoapReplyStatusCode status;

    Q_DECLARE_PUBLIC(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_P_H
