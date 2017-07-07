#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include <QtMath>

// TODO : remove pointer for message ?
QCoapInternalMessagePrivate::QCoapInternalMessagePrivate() :
    message(new QCoapMessage),
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate
    (const QCoapInternalMessagePrivate& other) :
    message(new QCoapMessage(*(other.message))),
    currentBlockNumber(other.currentBlockNumber),
    hasNextBlock(other.hasNextBlock),
    blockSize(other.blockSize)
{
}

QCoapInternalMessagePrivate::~QCoapInternalMessagePrivate()
{
    delete message;
}

QCoapInternalMessage::QCoapInternalMessage() :
    d_ptr(new QCoapInternalMessagePrivate)
{
}

QCoapInternalMessage::QCoapInternalMessage(QCoapMessage* message) :
    QCoapInternalMessage()
{
    d_ptr->message = message;
}

/*QCoapInternalMessage::QCoapInternalMessage(const QCoapInternalMessage& other) :
    QCoapMessage(other)
{
}*/

QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd):
    d_ptr(&dd)
{
}

void QCoapInternalMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption option(name, value);
    addOption(option);
}

void QCoapInternalMessage::addOption(const QCoapOption& option)
{
    QCoapInternalMessagePrivate* d = d_func();

    d->message->addOption(option);
}

QCoapMessage* QCoapInternalMessage::message() const
{
    return d_func()->message;
}

uint QCoapInternalMessage::currentBlockNumber() const
{
    return d_func()->currentBlockNumber;
}

bool QCoapInternalMessage::hasNextBlock() const
{
    return d_func()->hasNextBlock;
}

uint QCoapInternalMessage::blockSize() const
{
    return d_func()->blockSize;
}

QCoapInternalMessagePrivate* QCoapInternalMessage::d_func() const
{
    return static_cast<QCoapInternalMessagePrivate*>(d_ptr);
}
