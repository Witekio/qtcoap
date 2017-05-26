#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>

class QCoapMessage
{
public:
    enum QCoapMessageType {
        CONFIRMABLE,
        NONCONFIRMABLE,
        ACKNOWLEDGMENT,
        RESET
    };

    QCoapMessage();

    QCoapMessageType type() const;
    void setType(const QCoapMessageType& type);
};

#endif // QCOAPMESSAGE_H
