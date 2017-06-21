#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QList>
#include <qcoapoption.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate;
class QCoapMessage
{
public:
    enum QCoapMessageType {
        CONFIRMABLE,
        NONCONFIRMABLE,
        ACKNOWLEDGMENT,
        RESET
    };

    QCoapMessage();

    quint8 version() const;
    QCoapMessageType type() const;
    QByteArray token() const;
    quint8 tokenLength() const;
    quint16 messageId() const;
    QByteArray payload() const;
    QCoapOption option(int index) const;
    int optionsLength() const;
    void setVersion(quint8 version);
    void setType(const QCoapMessageType& type);
    void setToken(const QByteArray& token);
    void setMessageId(quint16);
    void setPayload(const QByteArray& payload);

    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value);
    void addOption(const QCoapOption& option);
    void removeOption(const QCoapOption& option);
    void removeOptionByName(QCoapOption::QCoapOptionName name);
    void removeAllOptions();

protected:
    QCoapMessage(QCoapMessagePrivate &dd);

    QCoapMessagePrivate* d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
