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

#ifndef QCOAPINTERNALREQUEST_H
#define QCOAPINTERNALREQUEST_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapinternalmessage.h>
#include <QtCoap/qcoapconnection.h>
#include <QtCore/qglobal.h>
#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
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

class QCoapRequest;
class QCoapInternalRequestPrivate;
class Q_AUTOTEST_EXPORT QCoapInternalRequest : public QCoapInternalMessage
{
    Q_OBJECT
public:
    enum OperationInternal {
        Empty,
        Get,
        Post,
        Put,
        Delete,
        Other
    };
    Q_ENUM(OperationInternal)

    explicit QCoapInternalRequest(QObject *parent = nullptr);
    explicit QCoapInternalRequest(const QCoapRequest &request, QObject *parent = nullptr);

    void initForAcknowledgment(quint16 messageId, const QByteArray &token);
    void initForReset(quint16 messageId);

    QByteArray toQByteArray() const;
    quint16 generateMessageId();
    QByteArray generateToken();
    void setRequestToAskBlock(uint blockNumber, uint blockSize);
    void setRequestToSendBlock(uint blockNumber, uint blockSize);

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption &option);
    void addUriOptions(const QUrl &uri, const QUrl &proxyUri = QUrl());

    QUrl targetUri() const;
    OperationInternal operation() const;
    bool cancelObserve() const;
    QCoapConnection *connection() const;
    uint retransmissionCounter() const;
    void setOperation(OperationInternal operation);
    void setConnection(QCoapConnection *connection);
    void setCancelObserve(bool cancelObserve);

    void setTargetUri(QUrl targetUri);
    void setTimeout(uint timeout);
    void beginTransmission();
    void stopTransmission();

    // To support use as keys in QMap
    bool operator<(const QCoapInternalRequest &other) const;

Q_SIGNALS:
    void timeout(QCoapInternalRequest*);

private:
    Q_DECLARE_PRIVATE(QCoapInternalRequest)
    Q_PRIVATE_SLOT(d_func(), void _q_timeout())
};

class Q_AUTOTEST_EXPORT QCoapInternalRequestPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalRequestPrivate();

    QUrl targetUri;
    QCoapInternalRequest::OperationInternal operation;
    QCoapConnection *connection;
    bool cancelObserve;
    QByteArray fullPayload;

    uint retransmissionCounter;
    int timeout;
    QTimer *timer = nullptr;

    void _q_timeout();

    Q_DECLARE_PUBLIC(QCoapInternalRequest)
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
