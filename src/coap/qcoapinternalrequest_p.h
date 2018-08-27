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

    bool isValid() const Q_DECL_OVERRIDE;

    void initForAcknowledgment(quint16 messageId, const QByteArray &token);
    void initForReset(quint16 messageId);

    QByteArray toQByteArray() const;
    quint16 generateMessageId();
    QCoapToken generateToken();
    void setToRequestBlock(int blockNumber, int blockSize);
    void setToSendBlock(int blockNumber, int blockSize);

    using QCoapInternalMessage::addOption;
    void addOption(const QCoapOption &option) Q_DECL_OVERRIDE;
    bool addUriOptions(QUrl uri, const QUrl &proxyUri = QUrl());

    QCoapToken token() const;
    QUrl targetUri() const;
    QtCoap::Method method() const;
    bool isObserve() const;
    bool isObserveCancelled() const;
    QCoapConnection *connection() const;
    int retransmissionCounter() const;
    void setMethod(QtCoap::Method method);
    void setConnection(QCoapConnection *connection);
    void setObserveCancelled();

    void setTargetUri(QUrl targetUri);
    void setTimeout(uint timeout);
    void setMaxTransmissionWait(int timeout);
    void restartTransmission();
    void stopTransmission();

Q_SIGNALS:
    void timeout(QCoapInternalRequest*);
    void maxTransmissionSpanReached(QCoapInternalRequest*);

protected:
    QCoapOption uriHostOption(const QUrl &uri) const;
    QCoapOption blockOption(QCoapOption::OptionName name, uint blockNumber, uint blockSize) const;

private:
    Q_DECLARE_PRIVATE(QCoapInternalRequest)
    Q_PRIVATE_SLOT(d_func(), void _q_timeout())
    Q_PRIVATE_SLOT(d_func(), void _q_maxTransmissionSpanReached())
};

class Q_AUTOTEST_EXPORT QCoapInternalRequestPrivate : public QCoapInternalMessagePrivate
{
public:
    QCoapInternalRequestPrivate() = default;

    QUrl targetUri;
    QtCoap::Method method = QtCoap::Invalid;
    QCoapConnection *connection = nullptr;
    QByteArray fullPayload;

    int timeout = 0;
    int retransmissionCounter = 0;
    QTimer *timeoutTimer = nullptr;
    QTimer *maxTransmitWaitTimer = nullptr;

    bool observeCancelled = false;
    bool transmissionInProgress = false;

    void _q_timeout();
    void _q_maxTransmissionSpanReached();

    Q_DECLARE_PUBLIC(QCoapInternalRequest)
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALREQUEST_H
