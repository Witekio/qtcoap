/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qdebug.h>
#include "qcoapoption_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCoapOption
    \brief The QCoapOption class holds data about CoAP options.

    \reentrant

    An option contains a name, related to an option id, and a value.
    The name is one of the values from the OptionName enumeration.
*/

/*!
    \enum QCoapOption::OptionName

    Indicates the name of an option.
    The value of each ID is as specified by the CoAP standard, with the
    exception of Invalid. You can refer to the
    \l{https://tools.ietf.org/html/rfc7252#section-5.10}{RFC 7252} for more
    details.

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
    and \a value. If no parameters are passed, constructs
    an Invalid object.

    \sa isValid()
 */
QCoapOption::QCoapOption(OptionName name,
                         const QByteArray &value) :
    d_ptr(new QCoapOptionPrivate)
{
    d_ptr->name = name;
    setValue(value);
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
int QCoapOption::length() const
{
    return d_ptr->value.length();
}

/*!
    Returns the name of the option.
 */
QCoapOption::OptionName QCoapOption::name() const
{
    return d_ptr->name;
}

/*!
    Returns \c true if the option is valid.
 */
bool QCoapOption::isValid() const
{
    return d_ptr->name != QCoapOption::Invalid;
}

/*!
    Returns \c true if this QCoapOption and \a other are equals.
 */
bool QCoapOption::operator==(const QCoapOption &other) const
{
    return (d_ptr->name == other.d_ptr->name
            && d_ptr->value == other.d_ptr->value);
}

/*!
    Returns \c true if this QCoapOption and \a other are different.
 */
bool QCoapOption::operator!=(const QCoapOption &other) const
{
    return !(*this == other);
}

/*!
    Sets the value for the option
 */
void QCoapOption::setValue(const QByteArray &value)
{
    bool oversized = false;

    // Check for value maximum size, according to section 5.10 of RFC 7252
    // https://tools.ietf.org/html/rfc7252#section-5.10
    switch (d_ptr->name) {
    case IfNoneMatch:
        if (value.size() > 0)
            oversized = true;
        break;

    case UriPort:
    case ContentFormat:
    case Accept:
        if (value.size() > 2)
            oversized = true;
        break;

    case MaxAge:
    case Size1:
        if (value.size() > 4)
            oversized = true;
        break;

    case IfMatch:
    case Etag:
        if (value.size() > 8)
            oversized = true;
        break;

    case UriHost:
    case LocationPath:
    case UriPath:
    case UriQuery:
    case LocationQuery:
    case ProxyScheme:
        if (value.size() > 255)
            oversized = true;
        break;

    case ProxyUri:
        if (value.size() > 1034)
            oversized = true;
        break;

    case Observe:
    case Block2:
    case Block1:
    case Size2:
    default:
        break;
    }

    if (oversized)
        qWarning() << "QCoapOption::setValue: value is probably too big for option" << d_ptr->name;

    d_ptr->value = value;
}

QT_END_NAMESPACE
