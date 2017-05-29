#ifndef QCOAPMESSAGE_P_H
#define QCOAPMESSAGE_P_H

#include <qcoapmessage.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QCoapMessagePrivate : public QObjectPrivate
{
public:
    QCoapMessagePrivate();

    Q_DECLARE_PUBLIC(QCoapMessage)
};

QT_END_NAMESPACE

#endif // QCOAPMESSAGE_P_H
