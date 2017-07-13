#ifndef QCOAPOPTION_P_H
#define QCOAPOPTION_P_H

#include "qcoapoption.h"
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QCoapOptionPrivate : public QObjectPrivate
{
public:
    QCoapOptionPrivate();

    QCoapOption::QCoapOptionName name;
    // TODO : remove length ?
    quint8 length;
    QByteArray value;
};

QT_END_NAMESPACE

#endif // QCOAPOPTION_P_H
