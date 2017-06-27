#ifndef QCOAPMESSAGE_P_H
#define QCOAPMESSAGE_P_H

#include "qcoapmessage.h"
#include <private/qobject_p.h>
#include <QSharedData>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate : public QSharedData
{
public:
    QCoapMessagePrivate();
    QCoapMessagePrivate(const QCoapMessagePrivate& other);
    ~QCoapMessagePrivate();

    quint8 version;
    QCoapMessage::QCoapMessageType type;
    quint16 messageId;
    QByteArray token;
    //quint8 tokenLength;
    QList<QCoapOption> options;
    QByteArray payload;
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_P_H
