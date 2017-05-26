#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include "qcoapmessage.h"
#include <QByteArray>

class QCoapReply : public QCoapMessage
{
public:
    enum QCoapReplyStatusCode {
        CREATED, // 2.01
        DELETED, // 2.02
        VALID,   // 2.03
        CHANGED, // 2.04
        CONTENT  // 2.05
    };

    QCoapReply();

    void fromPdu(const QByteArray& pdu);
    QByteArray readData();

    QCoapReplyStatusCode statusCode() const;
    QByteArray payload() const;
};

#endif // QCOAPREPLY_H
