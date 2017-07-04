#ifndef QCOAPINTERNALMESSAGE_P_H
#define QCOAPINTERNALMESSAGE_P_H

#include "qcoapinternalmessage.h"
#include "qcoapmessage_p.h"

class QCoapInternalMessagePrivate : public QCoapMessagePrivate
{
public:
    QCoapInternalMessagePrivate();
    QCoapInternalMessagePrivate(const QCoapInternalMessagePrivate& other);

    uint currentBlockNumber;
    bool hasNextBlock;
    uint blockSize;
};

#endif // QCOAPINTERNALMESSAGE_P_H