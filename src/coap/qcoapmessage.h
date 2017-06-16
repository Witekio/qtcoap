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
    QCoapMessageType type() const;
    QByteArray token() const;
    quint8 tokenLength() const;
    quint16 messageId() const;
    QByteArray payload() const;
    QCoapOption* option(int index) const;
    uint currentBlockNumber() const;
    bool hasNextBlock() const;
    int optionsLength() const;
    void setVersion(quint8 version);
    void setType(const QCoapMessageType& type);
    void setToken(const QByteArray& token);
    void setMessageId(quint16);
    void setPayload(const QByteArray& payload);

    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value);
    void addOption(QCoapOption* option);
    void removeOption(QCoapOption* option);
    void removeOptionByName(QCoapOption::QCoapOptionName name);
    void removeAllOptions();

protected:
    Q_DECLARE_PRIVATE(QCoapMessage)
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
