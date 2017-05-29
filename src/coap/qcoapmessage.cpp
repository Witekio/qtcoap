#include "qcoapmessage.h"
#include "qcoapmessage_p.h"

QCoapMessagePrivate::QCoapMessagePrivate()
{
}

QCoapMessage::QCoapMessage(QObject* parent) :
    QObject(parent)
{
}

QCoapMessage::QCoapMessageType QCoapMessage::type() const
{
    //TODO
}

void QCoapMessage::setType(const QCoapMessageType& type)
{
    Q_UNUSED(type);
    //TODO
}

qint64 QCoapMessage::token() const
{
    // TODO
    return 0;
}

qint16 QCoapMessage::messageId() const
{
    // TODO
    return 0;
}

QByteArray QCoapMessage::payload() const
{
    return QByteArray("");
}
