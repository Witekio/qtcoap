#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include <QtMath>

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate() :
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate
    (const QCoapInternalMessagePrivate& other) :
    QCoapMessagePrivate(other),
    currentBlockNumber(other.currentBlockNumber),
    hasNextBlock(other.hasNextBlock),
    blockSize(other.blockSize)
{
}

QCoapInternalMessage::QCoapInternalMessage() :
    QCoapMessage(*new QCoapInternalMessagePrivate)
{
}

QCoapInternalMessage::QCoapInternalMessage(const QCoapInternalMessage& other) :
    QCoapMessage(other)
{
}

QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd):
    QCoapMessage(dd)
{
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
