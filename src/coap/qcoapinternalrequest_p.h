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

#ifndef QCOAPINTERNALREQUEST_H
#define QCOAPINTERNALREQUEST_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoapinternalmessage.h>
#include <QtCoap/qcoapconnection.h>
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
    explicit QCoapInternalRequest(QObject *parent = nullptr);
    explicit QCoapInternalRequest(const QCoapRequest &request, QObject *parent = nullptr);

    void initForAcknowledgement(quint16 messageId, const QByteArray &token);
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
    QtCoap::Operation operation() const;
    bool cancelObserve() const;
    QCoapConnection *connection() const;
    uint retransmissionCounter() const;
    void setOperation(QtCoap::Operation operation);
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
    QCoapInternalRequestPrivate() = default;

    QUrl targetUri;
    QtCoap::Operation operation = QtCoap::Empty;
    QCoapConnection *connection = nullptr;
    QByteArray fullPayload;
    bool cancelObserve = false;

    int timeout = 0;
    uint retransmissionCounter = 0;
    QTimer *timer = nullptr;

    void _q_timeout();

    Q_DECLARE_PUBLIC(QCoapInternalRequest)
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
