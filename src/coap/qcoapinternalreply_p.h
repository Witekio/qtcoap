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

#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapinternalmessage.h>
#include <private/qcoapinternalmessage_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QCoapInternalReplyPrivate;
class Q_AUTOTEST_EXPORT QCoapInternalReply : public QCoapInternalMessage
{
    Q_OBJECT
public:
    enum StatusCodeInternal {
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
    Q_ENUM(StatusCodeInternal)

    explicit QCoapInternalReply(QObject *parent = nullptr);
    QCoapInternalReply(const QCoapInternalReply &other, QObject *parent = nullptr);

    static QCoapInternalReply fromQByteArray(const QByteArray &reply);
    void appendData(const QByteArray &data);
    int wantNextBlock();

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption &option);

    StatusCodeInternal statusCode() const;

private:
    Q_DECLARE_PRIVATE(QCoapInternalReply)
};

class Q_AUTOTEST_EXPORT QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate();

    QCoapInternalReply::StatusCodeInternal statusCode;
};

Q_DECLARE_METATYPE(QCoapInternalReply)

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
