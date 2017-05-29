#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>
#include <QList>
#include <qcoapoption.h>

class Q_DECL_EXPORT QCoapMessage
{
public:
    enum QCoapMessageType {
        CONFIRMABLE,
        NONCONFIRMABLE,
        ACKNOWLEDGMENT,
        RESET
    };

    QCoapMessage();

    QCoapMessageType type() const;
    void setType(const QCoapMessageType& type);
    qint64 token() const;
    qint16 messageId() const;
    QByteArray payload() const;

protected:
    QCoapMessageType type_p;
    qint16 messageId_p;
    qint64 token_p;
    QList<QCoapOption> options_p;
    QByteArray payload_p;
};

#endif // QCOAPMESSAGE_H
