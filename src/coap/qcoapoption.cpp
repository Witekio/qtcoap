/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
 */
QCoapOption::QCoapOption(OptionName name,
                         const QByteArray &value) :
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
QCoapOption::OptionName QCoapOption::name() const
{
    return d_ptr->name;
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

QT_END_NAMESPACE
