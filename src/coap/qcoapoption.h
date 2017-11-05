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

#ifndef QCOAPOPTION_H
#define QCOAPOPTION_H

#include <QtCoap/qcoapglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QCoapOptionPrivate;
class Q_COAP_EXPORT QCoapOption
{
public:
    enum OptionName {
        Invalid         = 0,
        IfMatch         = 1,
        UriHost         = 3,
        Etag            = 4,
        IfNoneMatch     = 5,
        Observe         = 6,
        UriPort         = 7,
        LocationPath    = 8,
        UriPath         = 11,
        ContentFormat   = 12,
        MaxAge          = 14,
        UriQuery        = 15,
        Accept          = 17,
        LocationQuery   = 20,
        Block2          = 23,
        Block1          = 27,
        Size2           = 28,
        ProxyUri        = 35,
        ProxyScheme     = 39,
        Size1           = 60
    };

    QCoapOption(OptionName name = Invalid, const QByteArray &value = QByteArray());

    QByteArray value() const;
    quint8 length() const;
    OptionName name() const;

    bool operator==(const QCoapOption &other) const;

protected:
    QCoapOptionPrivate *d_ptr;
};

Q_DECLARE_METATYPE(QCoapOption::OptionName)

QT_END_NAMESPACE

#endif // QCOAPOPTION_H
