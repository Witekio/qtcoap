#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include "qcoapglobal.h"
#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"

QT_BEGIN_NAMESPACE

class QCoapInternalReplyPrivate;
class QCoapInternalReply : public QCoapInternalMessage
{
    Q_OBJECT
public:
    QCoapInternalReply(QObject* parent = Q_NULLPTR);
    QCoapInternalReply(const QCoapInternalReply& other, QObject* parent = Q_NULLPTR);

    static QCoapInternalReply fromQByteArray(const QByteArray& reply);
    void appendData(const QByteArray& data);
    int wantNextBlock();

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption& option);

    QCoapStatusCode statusCode() const;

private:
    Q_DECLARE_PRIVATE(QCoapInternalReply)
};

class QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate();
    QCoapInternalReplyPrivate(const QCoapInternalReplyPrivate& other);

    QCoapStatusCode statusCode;
};

Q_DECLARE_METATYPE(QCoapInternalReply);

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
