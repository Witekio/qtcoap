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

#include <QtCoap/qcoapglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qrandom.h>

#ifndef QCOAPNAMESPACE_H
#define QCOAPNAMESPACE_H

QT_BEGIN_NAMESPACE

#define FOR_EACH_COAP_ERROR(X) \
    X(BadRequest, 0x80) X(Unauthorized, 0x81) X(BadOption, 0x82) X(Forbidden, 0x83)  \
    X(NotFound, 0x84) X(MethodNotAllowed, 0x85) X(NotAcceptable, 0x86) \
    X(RequestEntityIncomplete, 0x88) X(PreconditionFailed, 0x8C) X(RequestEntityTooLarge, 0x8D) \
    X(UnsupportedContentFormat, 0x8E) X(InternalServerError, 0xA0) X(NotImplemented, 0xA1) \
    X(BadGateway, 0xA2) X(ServiceUnavailable, 0xA3) X(GatewayTimeout, 0xA4) \
    X(ProxyingNotSupported, 0xA5)

class Q_COAP_EXPORT QtCoap : public QObject
{
    Q_OBJECT

private:
    QtCoap() {}

public:
    enum ResponseCode {
        EmptyMessage = 0x00,
        Created = 0x41, // 2.01
        Deleted = 0x42, // 2.02
        Valid   = 0x43, // 2.03
        Changed = 0x44, // 2.04
        Content = 0x45, // 2.05
        Continue = 0x5F, // 2.31

#define SINGLE_CODE(name, value) name = value,
        FOR_EACH_COAP_ERROR(SINGLE_CODE)
#undef SINGLE_CODE

        InvalidCode = 0xFF
    };
    Q_ENUM(ResponseCode)

    enum Error {
        NoError,
        HostNotFoundError,
        AddressInUseError,
        TimeOutError,

#define SINGLE_ERROR(name, ignored) name ## Error,
        FOR_EACH_COAP_ERROR(SINGLE_ERROR)
#undef SINGLE_ERROR

        UnknownError
    };
    Q_ENUM(Error)

    enum Method {
        Invalid,
        Get,
        Post,
        Put,
        Delete,
#if 0
        //! TODO Support other methods included in RFC 8132
        //! https://tools.ietf.org/html/rfc8132
        Fetch,
        Patch,
        IPatch,
#endif
        Other
    };
    Q_ENUM(Method)

    static bool isError(ResponseCode code)
    {
        return static_cast<int>(code) >= 0x80;
    }
    static Error responseCodeError(ResponseCode code);

    static QRandomGenerator randomGenerator;
};

Q_DECLARE_METATYPE(QtCoap::ResponseCode)
Q_DECLARE_METATYPE(QtCoap::Error)
Q_DECLARE_METATYPE(QtCoap::Method)

QT_END_NAMESPACE

#endif // QCOAPNAMESPACE_H
