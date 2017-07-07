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
        ConfirmableMessage,
        NonConfirmableMessage,
        AcknowledgmentMessage,
        ResetMessage
    };

    QCoapMessage();
    QCoapMessage(const QCoapMessage& other);
    virtual ~QCoapMessage();

    quint8 version() const;
    QCoapMessageType type() const;
    QByteArray token() const;
    quint8 tokenLength() const;
    quint16 messageId() const;
    QByteArray payload() const;
    QCoapOption option(int index) const;
    QList<QCoapOption> optionList() const;
    int optionsLength() const;
    void setVersion(quint8 version);
    void setType(const QCoapMessageType& type);
    void setToken(const QByteArray& token);
    void setMessageId(quint16);
    void setPayload(const QByteArray& payload);

    void addOption(QCoapOption::QCoapOptionName name, const QByteArray& value = QByteArray());
    virtual void addOption(const QCoapOption& option);
    QCoapOption findOptionByName(QCoapOption::QCoapOptionName name);
    void removeOption(const QCoapOption& option);
    void removeOptionByName(QCoapOption::QCoapOptionName name);
    void removeAllOptions();

protected:
    QCoapMessage(QCoapMessagePrivate &dd);

    QCoapMessagePrivate* d_ptr;
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
