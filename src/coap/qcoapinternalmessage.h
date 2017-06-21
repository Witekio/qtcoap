#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include "qcoapmessage.h"

QT_BEGIN_NAMESPACE

class QCoapInternalMessagePrivate;
class QCoapInternalMessage : public QCoapMessage
{
public:
    QCoapInternalMessage();

    void addOption(const QCoapOption& option);

    uint currentBlockNumber() const;
    bool hasNextBlock() const;
    uint blockSize() const;

protected:
    QCoapInternalMessagePrivate* d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALMESSAGE_H
