#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include "qcoapmessage.h"

QT_BEGIN_NAMESPACE

class QCoapInternalMessagePrivate;
class QCoapInternalMessage //: public QCoapMessage
{
public:
    QCoapInternalMessage();
    QCoapInternalMessage(QCoapMessage* message);
    //QCoapInternalMessage(const QCoapInternalMessage& other);
    virtual ~QCoapInternalMessage() {}

    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value);
    virtual void addOption(const QCoapOption& option);

    QCoapMessage* message() const;
    uint currentBlockNumber() const;
    bool hasNextBlock() const;
    uint blockSize() const;

protected:
    QCoapInternalMessage(QCoapInternalMessagePrivate &dd);
    QCoapInternalMessagePrivate* d_func() const;
    QCoapInternalMessagePrivate* d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALMESSAGE_H
