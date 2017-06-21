#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include <QtMath>

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate() :
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapInternalMessage::QCoapInternalMessage() :
    QCoapMessage(*new QCoapInternalMessagePrivate)
{
}

QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd):
    QCoapMessage(dd)
{
}

void QCoapInternalMessage::addOption(const QCoapOption& option)
{
    QCoapInternalMessagePrivate* d = d_func();
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::BLOCK1
        || option.name() == QCoapOption::BLOCK2) {
        quint32 blockNumber = 0;
        quint8 *optionData = (quint8 *)option.value().data();
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d->blockSize = qPow(2, (optionData[option.length()-1] & 0x7) + 4);
        //qDebug() << option->length();
        /*qDebug() << "ADD BLOCK : " << d->currentBlockNumber
                 << " - " << d->hasNextBlock
                 << " - " << option->value().toHex();*/
    }

    QCoapMessage::addOption(option);
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
