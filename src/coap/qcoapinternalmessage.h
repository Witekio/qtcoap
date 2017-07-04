#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include "qcoapmessage.h"

QT_BEGIN_NAMESPACE

class QCoapInternalMessagePrivate;
class QCoapInternalMessage : public QCoapMessage
{
public:
    QCoapInternalMessage();
    QCoapInternalMessage(const QCoapInternalMessage& other);

    uint currentBlockNumber() const;
    bool hasNextBlock() const;
    uint blockSize() const;

protected:
    QCoapInternalMessage(QCoapInternalMessagePrivate &dd);
    QCoapInternalMessagePrivate* d_func() const;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALMESSAGE_H