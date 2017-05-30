#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include "qcoapmessage.h"
#include <QByteArray>

QT_BEGIN_NAMESPACE

class QCoapReplyPrivate;
class QCoapReply : public QCoapMessage
{
public:
    enum QCoapReplyStatusCode {
        CREATED, // 2.01
        DELETED, // 2.02
        VALID,   // 2.03
        CHANGED, // 2.04
        CONTENT  // 2.05
        // TODO : find the corresponding hexadecimal code and add other status code
    };

    QCoapReply();

    void fromPdu(const QByteArray& pdu);
    QByteArray readData();

    QCoapReplyStatusCode statusCode() const;

protected:
    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_H
