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

    QCoapMessage(QObject* parent = nullptr);

    QCoapMessageType type() const;
    void setType(const QCoapMessageType& type);
    qint64 token() const;
    qint16 messageId() const;
    QByteArray payload() const;

protected:
    Q_DECLARE_PRIVATE(QCoapMessage)
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_H
