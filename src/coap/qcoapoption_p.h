#ifndef QCOAPOPTION_P_H
#define QCOAPOPTION_P_H

#include "qcoapoption.h"
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QCoapOptionPrivate : public QObjectPrivate
{
public:
    QCoapOptionPrivate();

    Q_DECLARE_PUBLIC(QCoapOption);
};

QT_END_NAMESPACE

#endif // QCOAPOPTION_P_H