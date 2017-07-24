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

/*!
    \internal

    \class QCoapInternalMessage
    \brief The QCoapInternalMessage class contains data related to
    a received message or a message to send. It contains an instance of
    QCoapMessage and other data for the block management.

    \reentrant

    The QCoapInternalMessage class is inherited by QCoapInternalRequest and
    QCoapInternalReply that are used internally to manage request to send
    and receive replies.

    \sa QCoapInternalReply, QCoapInternalRequest, QCoapMessage
*/

/*!
    \internal

    Constructs a new QCoapInternalMessage and sets \a parent as the parent
    object.
 */
QCoapInternalMessage::QCoapInternalMessage(QObject* parent) :
    QObject(* new QCoapInternalMessagePrivate, parent)
{
}

/*!
    \internal

    Constructs a new QCoapInternalMessage with the given \a message and sets
    \a parent as the parent object.
 */
QCoapInternalMessage::QCoapInternalMessage(const QCoapMessage& message, QObject* parent) :
    QCoapInternalMessage(parent)
{
    Q_D(QCoapInternalMessage);
    d->message = message;
}

/*!
    \internal

    Constructs a copy of \a other and sets \a parent as the parent object.
*/
QCoapInternalMessage::QCoapInternalMessage(const QCoapInternalMessage& other, QObject* parent) :
    QObject(* new QCoapInternalMessagePrivate(*other.d_func()), parent)
{
}

/*!
    \internal
    Constructs a new QCoapInternalMessage with \a dd as the d_ptr.
    This constructor must be used when subclassing internally
    the QCoapInternalMessage class.
*/
QCoapInternalMessage::QCoapInternalMessage(QCoapInternalMessagePrivate &dd, QObject* parent):
    QObject(dd, parent)
{
}

/*!
    \internal
    \overload

    Adds the coap option with the given \a name and \a value.
*/
void QCoapInternalMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption option(name, value);
    addOption(option);
}

/*!
    \internal

    Adds the given coap \a option.
*/
void QCoapInternalMessage::addOption(const QCoapOption& option)
{
    Q_D(QCoapInternalMessage);
    d->message.addOption(option);
}

/*!
    \internal

    Removes the option with the given \a name.
*/
void QCoapInternalMessage::removeOptionByName(QCoapOption::QCoapOptionName name)
{
    Q_D(QCoapInternalMessage);
    d->message.removeOptionByName(name);
}

/*!
    \internal

    Returns the message instance.
*/
QCoapMessage QCoapInternalMessage::message() const
{
    return d_func()->message;
}

/*!
    \internal

    Returns the block number
*/
uint QCoapInternalMessage::currentBlockNumber() const
{
    return d_func()->currentBlockNumber;
}

/*!
    \internal

    Returns true if it has next block.
    And false if it is the last block.
*/
bool QCoapInternalMessage::hasNextBlock() const
{
    return d_func()->hasNextBlock;
}

/*!
    \internal

    Returns the size of the block.
*/
uint QCoapInternalMessage::blockSize() const
{
    return d_func()->blockSize;
}

QT_END_NAMESPACE
