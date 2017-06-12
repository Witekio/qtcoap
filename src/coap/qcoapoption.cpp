#include "qcoapoption.h"
#include "qcoapoption_p.h"

QCoapOptionPrivate::QCoapOptionPrivate() :
    length(0),
    value(QByteArray())
{
}

QCoapOption::QCoapOption(QCoapOptionName name,
                         const QByteArray& value,
                         QObject* parent) :
    QObject(* new QCoapOptionPrivate, parent)
{
    Q_D(QCoapOption);
    d->name = name;
    d->value = value;
    d->length = quint8(value.length());
}

QByteArray QCoapOption::value() const
{
    return d_func()->value;
}

quint8 QCoapOption::length() const
{
    return d_func()->length;
}

QCoapOption::QCoapOptionName QCoapOption::name() const
{
    return d_func()->name;
}
