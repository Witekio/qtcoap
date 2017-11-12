/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
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

#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include <QtCoap/qcoapprotocol.h>
#include <QtCore/qlist.h>
#include <QtCore/qpair.h>
#include <QtCore/qqueue.h>
#include <QtCore/qpointer.h>
#include <private/qobject_p.h>

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

struct InternalMessagePair {
    QPointer<QCoapReply> userReply;
    QList<QCoapInternalReply*> replies;
};

typedef QMap<QCoapInternalRequest*, InternalMessagePair> InternalMessageMap;

class Q_AUTOTEST_EXPORT QCoapProtocolPrivate : public QObjectPrivate
{
public:
    QCoapProtocolPrivate() = default;

    void handleFrame(const QByteArray &frame);
    void onLastBlock(QCoapInternalRequest *request);
    void onNextBlock(QCoapInternalRequest *request, uint currentBlockNumber, uint blockSize);
    void sendAcknowledgement(QCoapInternalRequest *request);
    void sendReset(QCoapInternalRequest *request);
    QByteArray encode(QCoapInternalRequest *request);
    QCoapInternalReply *decode(const QByteArray &message);

    void sendRequest(QCoapInternalRequest *request);
    void resendRequest(QCoapInternalRequest *request);
    bool containsMessageId(quint16 id);
    bool containsToken(const QByteArray &token);
    QCoapInternalRequest *findInternalRequestByToken(const QByteArray &token);
    QCoapInternalRequest *findInternalRequestByMessageId(quint16 messageId);
    QCoapInternalRequest *findInternalRequestByReply(QCoapReply *reply);

    void messageReceived(const QByteArray &frameReply);
    void onAbortedRequest(QCoapReply *reply);

    InternalMessageMap internalReplies;
    QQueue<QByteArray> frameQueue;
    quint16 blockSize = 0;

    uint ackTimeout = 2000;
    double ackRandomFactor = 1.5;
    uint maxRetransmit = 4;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
