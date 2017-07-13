#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include "qcoapmessage.h"
#include <QObject>

QT_BEGIN_NAMESPACE

class QCoapInternalMessagePrivate;
class QCoapInternalMessage : public QObject
{
    Q_OBJECT
public:
    QCoapInternalMessage(QObject* parent = Q_NULLPTR);
    QCoapInternalMessage(const QCoapMessage& message, QObject* parent = Q_NULLPTR);
    QCoapInternalMessage(const QCoapInternalMessage& other, QObject* parent = Q_NULLPTR);
    virtual ~QCoapInternalMessage() {}

    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value);
    virtual void addOption(const QCoapOption& option);
    void removeOptionByName(QCoapOption::QCoapOptionName name);

    QCoapMessage message() const;
    uint currentBlockNumber() const;
    bool hasNextBlock() const;
    uint blockSize() const;

protected:
    QCoapInternalMessage(QCoapInternalMessagePrivate &dd, QObject* parent = Q_NULLPTR);
    Q_DECLARE_PRIVATE(QCoapInternalMessage)
    //QCoapInternalMessagePrivate* d_func() const;
    //QCoapInternalMessagePrivate* d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALMESSAGE_H
