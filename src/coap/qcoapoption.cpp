#include "qcoapoption.h"
#include "qcoapoption_p.h"

// TODO : add autotests for QCoapOption

QCoapOptionPrivate::QCoapOptionPrivate() :
    length_p(0),
    value_p(QByteArray())
{
}

QCoapOption::QCoapOption(QCoapOptionName name,
                         const QByteArray& value,
                         QObject* parent) :
    QObject(* new QCoapOptionPrivate, parent)
{
    Q_D(QCoapOption);
    d->name_p = name;
    d->value_p = value;
    d->length_p = quint8(value.length());
}

QByteArray QCoapOption::value() const
{
    return d_func()->value_p;
}
quint8 QCoapOption::length() const
{
    return d_func()->length_p;
}
QCoapOption::QCoapOptionName QCoapOption::name() const
{
    return d_func()->name_p;
}
