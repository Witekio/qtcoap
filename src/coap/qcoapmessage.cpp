#include "qcoapmessage.h"
#include "qcoapmessage_p.h"

QT_BEGIN_NAMESPACE

QCoapMessagePrivate::QCoapMessagePrivate() :
    QSharedData(),
    version(1),
    type(QCoapMessage::NonConfirmableCoapMessage),
    messageId(0),
    token(QByteArray()),
    payload(QByteArray())
{
}

QCoapMessagePrivate::QCoapMessagePrivate(const QCoapMessagePrivate& other) :
    QSharedData(other),
    version(other.version),
    type(other.type),
    messageId(other.messageId),
    token(other.token),
    options(other.options),
    payload(other.payload)
{
}

QCoapMessagePrivate::~QCoapMessagePrivate()
{
}


/*!
    \class QCoapMessage
    \brief The QCoapMessage class holds informations about a coap message that
    can be a request or a reply.

    \reentrant

    It holds informations as the message type, message id, token and other
    ancillary data.

    \sa QCoapOption, QCoapReply, QCoapRequest
*/

/*!
    \enum QCoapMessage::QCoapMessageType

    Indicates the type of the message.

    \value ConfirmableCoapMessage       A Confirmable message. The destination
                                        endpoint needs to acknowledge the
                                        message.
    \value NonConfirmableCoapMessage    A Non-Confirmable message. The
                                        destination endpoint does not need to
                                        acknowledge the message.
    \value AcknowledgmentCoapMessage    An Acknowledgment message. A message
                                        sent or received in reply to a
                                        Confirmable message.
    \value ResetCoapMessage             A Reset message. A message sent
                                        sometimes in case of errors or to stop
                                        transmissions. (For example, it is used
                                        to cancel an observation)
*/

/*!
    Constructs a new QCoapMessage.
*/
QCoapMessage::QCoapMessage() :
    d_ptr(new QCoapMessagePrivate)
{
}

/*!
    Constructs a copy of \a other.
*/
QCoapMessage::QCoapMessage(const QCoapMessage& other) :
    d_ptr(other.d_ptr)
{
}

/*!
    \internal
*/
QCoapMessage::QCoapMessage(QCoapMessagePrivate &dd) :
    d_ptr(&dd)
{
}

/*!
    \overload

    Adds the coap option with the given \a name and \a value.
*/
void QCoapMessage::addOption(QCoapOption::QCoapOptionName name, const QByteArray& value)
{
    QCoapOption option(name, value);
    addOption(option);
}

/*!
    Adds the given coap \a option.
*/
void QCoapMessage::addOption(const QCoapOption& option)
{
    d_ptr->options.push_back(option);
}

/*!
    Removes the given \a option.
*/
void QCoapMessage::removeOption(const QCoapOption& option)
{
    d_ptr->options.removeOne(option);
}

/*!
    Removes the option with the given \a name.
*/
void QCoapMessage::removeOptionByName(QCoapOption::QCoapOptionName name)
{
    for (QCoapOption option : d_ptr->options) {
        if (option.name() == name) {
            removeOption(option);
            break;
        }
    }
}

/*!
    Finds and returns the option with the given \a name.
    If no option found, returns an InvalidCoapOption with en empty value.
*/
QCoapOption QCoapMessage::findOptionByName(QCoapOption::QCoapOptionName name)
{
    for (QCoapOption option : d_ptr->options) {
        if (option.name() == name)
            return option;
    }

    return QCoapOption(QCoapOption::InvalidCoapOption, QByteArray());
}

/*!
    Removes all options.
*/
void QCoapMessage::removeAllOptions()
{
    d_ptr->options.clear();
}

/*!
    Returns the coap version.

    \sa setVersion()
*/
quint8 QCoapMessage::version() const
{
    return d_ptr->version;
}

/*!
    Returns the message type.

    \sa setType()
*/
QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    return d_ptr->type;
}

/*!
    Returns the message token.

    \sa setToken()
*/
QByteArray QCoapMessage::token() const
{
    return d_ptr->token;
}

/*!
    Returns the token length.
*/
quint8 QCoapMessage::tokenLength() const
{
    return static_cast<quint8>(d_ptr->token.length()); //tokenLength;
}

/*!
    Returns the message id.

    \sa setMessageId()
*/
quint16 QCoapMessage::messageId() const
{
    return d_ptr->messageId;
}

/*!
    Returns the payload.

    \sa setPayload()
*/
QByteArray QCoapMessage::payload() const
{
    return d_ptr->payload;
}

/*!
    Returns the option at \a index position.
*/
QCoapOption QCoapMessage::option(int index) const
{
    return d_ptr->options.at(index);
}

/*!
    Returns the list of options.
*/
QList<QCoapOption> QCoapMessage::optionList() const
{
    return d_ptr->options;
}

/*!
    Returns the number of options.
*/
int QCoapMessage::optionsLength() const
{
    return d_ptr->options.length();
}

/*!
    Sets the coap version.

    \sa version()
*/
void QCoapMessage::setVersion(quint8 version)
{
    if (d_ptr->version == version)
        return;

    d_ptr->version = version;
}

/*!
    Sets the message type.

    \sa type()
*/
void QCoapMessage::setType(const QCoapMessageType& type)
{
    if (d_ptr->type == type)
        return;

    d_ptr->type = type;
}

/*!
    Sets the token.

    \sa token()
*/
void QCoapMessage::setToken(const QByteArray& token)
{
    if (d_ptr->token == token)
        return;

    d_ptr->token = token;
}

/*!
    Sets the message id.

    \sa messageId()
*/
void QCoapMessage::setMessageId(quint16 id)
{
    if (d_ptr->messageId == id)
        return;

    d_ptr->messageId = id;
}

/*!
    Sets the payload.

    \sa payload()
*/
void QCoapMessage::setPayload(const QByteArray& payload)
{
    if (d_ptr->payload == payload)
        return;

    d_ptr->payload = payload;
}

QT_END_NAMESPACE
