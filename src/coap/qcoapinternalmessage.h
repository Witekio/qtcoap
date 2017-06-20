#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include "qcoapmessage.h"

class QCoapInternalMessagePrivate;
class QCoapInternalMessage : public QCoapMessage
{
public:
    QCoapInternalMessage();

protected:
    QCoapInternalMessagePrivate* d_ptr;
};

#endif // QCOAPINTERNALMESSAGE_H
