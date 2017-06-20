#include "qcoapoption.h"
#include "qcoapoption_p.h"

QCoapOptionPrivate::QCoapOptionPrivate() :
    length(0),
    value(QByteArray())
{
}

QCoapOption::QCoapOption(QCoapOptionName name,
                         const QByteArray& value) :
    d_ptr(new QCoapOptionPrivate)
{
    d_ptr->name = name;
    d_ptr->value = value;
    d_ptr->length = quint8(value.length());
}

QByteArray QCoapOption::value() const
{
    return d_ptr->value;
}

quint8 QCoapOption::length() const
{
    return d_ptr->length;
}

QCoapOption::QCoapOptionName QCoapOption::name() const
{
    return d_ptr->name;
}

bool QCoapOption::operator==(const QCoapOption& other) const
{
    return (d_ptr->name == other.d_ptr->name
            && d_ptr->value == other.d_ptr->value
            && d_ptr->length == other.d_ptr->length);
}
