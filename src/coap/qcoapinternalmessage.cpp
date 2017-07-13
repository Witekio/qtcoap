#include "qcoapinternalmessage.h"
#include "qcoapinternalmessage_p.h"
#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate() :
    currentBlockNumber(0),
    hasNextBlock(false),
    blockSize(0)
{
}

QCoapInternalMessagePrivate::QCoapInternalMessagePrivate
    (const QCoapInternalMessagePrivate& other) :
    QObjectPrivate (other),
    message(other.message),
    currentBlockNumber(other.currentBlockNumber),
    hasNextBlock(other.hasNextBlock),
    blockSize(other.blockSize)
{
}

QCoapInternalMessagePrivate::~QCoapInternalMessagePrivate()
{
}

QCoapInternalMessage::QCoapInternalMessage(QObject* parent) :
    QObject(* new QCoapInternalMessagePrivate, parent)
    //d_ptr(new QCoapInternalMessagePrivate)
{
}

QCoapInternalMessage::QCoapInternalMessage(const QCoapMessage& message, QObject* parent) :
    QCoapInternalMessage(parent)
{
    Q_D(QCoapInternalMessage);
    d->message = message;
}

QCoapInternalMessage::QCoapInternalMessage(const QCoapInternalMessage& other, QObject* parent) :
    QObject(* new QCoapInternalMessagePrivate(*other.d_func()), parent)
{
}

QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd, QObject* parent):
    QObject(dd, parent)
{
}

void QCoapInternalMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption option(name, value);
    addOption(option);
}

void QCoapInternalMessage::addOption(const QCoapOption& option)
{
    Q_D(QCoapInternalMessage);
    d->message.addOption(option);
}

void QCoapInternalMessage::removeOptionByName(QCoapOption::QCoapOptionName name)
{
    Q_D(QCoapInternalMessage);
    d->message.removeOptionByName(name);
}

QCoapMessage QCoapInternalMessage::message() const
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

QT_END_NAMESPACE
