#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include "qcoapmessage.h"
#include <QByteArray>

QT_BEGIN_NAMESPACE

class QCoapReplyPrivate;
class QCoapReply : public QCoapMessage
{
    Q_OBJECT
public:
    enum QCoapReplyStatusCode {
        INVALIDCODE = 0x00,
        CREATED = 0x41, // 2.01
        DELETED = 0x42, // 2.02
        VALID   = 0x43,   // 2.03
        CHANGED = 0x44, // 2.04
        CONTENT = 0x45 // 2.05
        // TODO : add other status code
    };

    QCoapReply(QObject* parent = Q_NULLPTR);

    void fromPdu(const QByteArray& pdu);
    QByteArray readData();

    QCoapReplyStatusCode statusCode() const;

protected:
    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_H
