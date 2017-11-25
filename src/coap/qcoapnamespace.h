/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL3$
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

#include <QtCoap/qcoapglobal.h>
#include <QtCore/qobject.h>

#ifndef QCOAPNAMESPACE_H
#define QCOAPNAMESPACE_H

QT_BEGIN_NAMESPACE

class Q_COAP_EXPORT QtCoap : public QObject
{
    Q_OBJECT

private:
    QtCoap() {}

public:
    enum StatusCode {
        Invalid = 0x00,
        Created = 0x41, // 2.01
        Deleted = 0x42, // 2.02
        Valid   = 0x43, // 2.03
        Changed = 0x44, // 2.04
        Content = 0x45, // 2.05
        Continue = 0x5F, // 2.31
        BadRequest = 0x80, // 4.00
        Unauthorized = 0x81, // 4.01
        BadOption = 0x82, // 4.02
        Forbidden = 0x83, // 4.03
        NotFound = 0x84, // 4.04
        MethodNotAllowed = 0x85, // 4.05
        NotAcceptable = 0x86, // 4.06
        RequestEntityIncomplete = 0x88, // 4.08
        PreconditionFailed = 0x8C, // 4.12
        RequestEntityTooLarge = 0x8D, // 4.13
        UnsupportedContentFormat = 0x8E, // 4.14
        InternalServerError = 0xA0, // 5.00
        NotImplemented = 0xA1, // 5.01
        BadGateway = 0xA2, // 5.02
        ServiceUnavailable = 0xA3, // 5.03
        GatewayTimeout = 0xA4, // 5.04
        ProxyingNotSupported = 0xA5 // 5.05
    };
    Q_ENUM(StatusCode)

    enum Operation {
        Empty,
        Get,
        Post,
        Put,
        Delete,
#if 0
        //! TODO, included in RFC 8132
        //! https://tools.ietf.org/html/rfc8132
        Fetch,
        Patch,
        IPatch,
#endif
        Other
    };
    Q_ENUM(Operation)
};

Q_DECLARE_METATYPE(QtCoap::Operation)

QT_END_NAMESPACE

#endif // QCOAPNAMESPACE_H
