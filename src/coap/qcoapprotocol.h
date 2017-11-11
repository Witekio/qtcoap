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

#ifndef QCOAPPROTOCOL_H
#define QCOAPPROTOCOL_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapconnection.h>
#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapresource.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QCoapInternalRequest;
class QCoapInternalReply;
class QCoapProtocolPrivate;
class Q_COAP_EXPORT QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = nullptr);

    uint ackTimeout() const;
    double ackRandomFactor() const;
    uint maxRetransmit() const;
    quint16 blockSize() const;
    void setAckTimeout(uint ackTimeout);
    void setAckRandomFactor(double ackRandomFactor);
    void setMaxRetransmit(uint maxRetransmit);
    void setBlockSize(quint16 blockSize);

    static QList<QCoapResource> resourcesFromCoreLinkList(const QByteArray &data);

Q_SIGNALS:
    void finished(QCoapReply*);

public Q_SLOTS:
    void sendRequest(QPointer<QCoapReply> reply, QCoapConnection *connection);
    void cancelObserve(QPointer<QCoapReply> reply);

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)
    Q_PRIVATE_SLOT(d_func(), void resendRequest(QCoapInternalRequest*))
    Q_PRIVATE_SLOT(d_func(), void sendRequest(QCoapInternalRequest*))
    Q_PRIVATE_SLOT(d_func(), void messageReceived(const QByteArray&))
    Q_PRIVATE_SLOT(d_func(), void onAbortedRequest(QCoapReply *reply))
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_H
