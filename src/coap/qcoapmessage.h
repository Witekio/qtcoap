#ifndef QCOAPMESSAGE_H
#define QCOAPMESSAGE_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QList>
#include <qcoapoption.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate;
class Q_DECL_EXPORT QCoapMessage : public QObject
{
    Q_OBJECT

public:
    enum QCoapMessageType {
        CONFIRMABLE,
        NONCONFIRMABLE,
        ACKNOWLEDGMENT,
        RESET
    };

    QCoapMessage(QObject* parent = Q_NULLPTR);
    QCoapMessage(QCoapMessagePrivate &dd, QObject* parent = Q_NULLPTR);
    //QCoapMessage(const QCoapMessage& other);

    quint8 version() const;
    void setVersion(quint8 version);
    QCoapMessageType type() const;
    void setType(const QCoapMessageType& type);
    QByteArray token() const;
    quint8 tokenLength() const;
    void setToken(const QByteArray& token);
    quint16 messageId() const;
    void setMessageId(quint16);
    QByteArray payload() const;
    void setPayload(const QByteArray& payload);
    QCoapOption* option(int index) const;
    int optionsLength() const;

    // TODO : add tests for addOption
    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value);
    void addOption(QCoapOption* option);

protected:
    Q_DECLARE_PRIVATE(QCoapMessage)
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
