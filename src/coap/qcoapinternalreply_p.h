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

#ifndef QCOAPINTERNALREPLY_H
#define QCOAPINTERNALREPLY_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoapinternalmessage.h>
#include <private/qcoapinternalmessage_p.h>
#include <QtNetwork/qhostaddress.h>

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
    explicit QCoapInternalReply(QObject *parent = nullptr);
    QCoapInternalReply(const QCoapInternalReply &other, QObject *parent = nullptr);

    static QCoapInternalReply *createFromFrame(const QByteArray &frame, QObject *parent = nullptr);
    void appendData(const QByteArray &data);
    int nextBlockToSend() const;

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption &option);
    void setSenderAddress(const QHostAddress &address);

    QtCoap::ResponseCode responseCode() const;
    QHostAddress senderAddress() const;

private:
    Q_DECLARE_PRIVATE(QCoapInternalReply)
};

class Q_AUTOTEST_EXPORT QCoapInternalReplyPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalReplyPrivate() = default;

    QtCoap::ResponseCode responseCode = QtCoap::InvalidCode;
    QHostAddress senderAddress;
};

Q_DECLARE_METATYPE(QCoapInternalReply)

QT_END_NAMESPACE

#endif // QCOAPINTERNALREPLY_H
