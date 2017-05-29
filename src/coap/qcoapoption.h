#ifndef QCOAPOPTION_H
#define QCOAPOPTION_H

#include <QObject>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapOptionPrivate;
class QCoapOption : public QObject
{
    Q_OBJECT

public:
    QCoapOption();

private:
    Q_DECLARE_PRIVATE(QCoapOption)
};

QT_END_NAMESPACE

#endif // QCOAPOPTION_H
