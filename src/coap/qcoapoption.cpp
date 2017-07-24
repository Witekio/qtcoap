#include "qcoapoption.h"
#include "qcoapoption_p.h"

QT_BEGIN_NAMESPACE

QCoapOptionPrivate::QCoapOptionPrivate() :
    value(QByteArray())
{
}

/*!
    \class QCoapOption
    \brief The QCoapOption class holds data about coap options.

    \reentrant

    An option contains a name, related to an option id, and a value.
    The name is set with an enumeration value : QCoapOptionName.
*/

/*!
    \enum QCoapOption::QCoapOptionName

    Indicates the name of the option.
    Its value is the id related to the name of the option
    as defined in the CoAP specifications.

    \value IfMatchCoapOption        If-match
    \value UriHostCoapOption        Uri-Host
    \value EtagCoapOption           Etag
    \value IfNoneMatchCoapOption    If-none-match
    \value ObserveCoapOption        Observe
    \value UriPortCoapOption        Uri-Port
    \value LocationPathCoapOption   Location path
    \value UriPathCoapOption        Uri-path
    \value ContentFormatCoapOption  Content-Format
    \value MaxAgeCoapOption         Max-Age
    \value UriQueryCoapOption       Uri-Query
    \value AcceptCoapOption         Accept
    \value LocationQueryCoapOption  Location Query
    \value Block2CoapOption         Block2
    \value Block1CoapOption         Block1
    \value Size2CoapOption          Size2
    \value ProxyUriCoapOption       Proxy-Uri
    \value ProxySchemeCoapOption    Proxy-Scheme
    \value Size1CoapOption          Size1
*/

/*!
    Constructs a QCoapOption object with the given \a name
    and \a value.
 */
QCoapOption::QCoapOption(QCoapOptionName name,
                         const QByteArray& value) :
    d_ptr(new QCoapOptionPrivate)
{
    d_ptr->name = name;
    d_ptr->value = value;
}

/*!
    Returns the value of the option.
 */
QByteArray QCoapOption::value() const
{
    return d_ptr->value;
}

/*!
    Returns the length of the value of the option.
 */
quint8 QCoapOption::length() const
{
    return quint8(d_ptr->value.length());
}

/*!
    Returns the name of the option.
 */
QCoapOption::QCoapOptionName QCoapOption::name() const
{
    return d_ptr->name;
}

/*!
    Returns true if this QCoapOption and \a other are equals.
 */
bool QCoapOption::operator==(const QCoapOption& other) const
{
    return (d_ptr->name == other.d_ptr->name
            && d_ptr->value == other.d_ptr->value);
}

QT_END_NAMESPACE
