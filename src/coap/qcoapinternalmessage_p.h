#ifndef QCOAPINTERNALMESSAGE_P_H
#define QCOAPINTERNALMESSAGE_P_H

#include "qcoapinternalmessage.h"
#include "qcoapmessage_p.h"
#include <private/qobject_p.h>

class QCoapInternalMessagePrivate : public QObjectPrivate //: public QCoapMessagePrivate
{
public:
    QCoapInternalMessagePrivate();
    QCoapInternalMessagePrivate(const QCoapInternalMessagePrivate& other);
    ~QCoapInternalMessagePrivate();

    QCoapMessage* message;

    uint currentBlockNumber;
    bool hasNextBlock;
    uint blockSize;
};

#endif // QCOAPINTERNALMESSAGE_P_H
