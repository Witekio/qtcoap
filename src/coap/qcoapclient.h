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

#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE

class QCoapReply;
class QCoapDiscoveryReply;
class QCoapRequest;
class QCoapProtocol;

class QCoapClientPrivate;
class Q_COAP_EXPORT QCoapClient : public QObject
{
    Q_OBJECT
public:
    explicit QCoapClient(QObject *parent = nullptr);
    ~QCoapClient();

    QCoapReply *get(const QCoapRequest &target);
    QCoapReply *put(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *put(const QCoapRequest &request, QIODevice *device);
    QCoapReply *post(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *post(const QCoapRequest &request, QIODevice *device);
    QCoapReply *deleteResource(const QCoapRequest &request);
    QCoapReply *observe(const QCoapRequest &request);
    void cancelObserve(const QCoapRequest &request);
    void cancelObserve(QCoapReply *notifiedReply);

    QCoapDiscoveryReply *discover(const QUrl &url,
                                  const QString &discoveryPath = QLatin1String("/.well-known/core"));

    void setBlockSize(quint16 blockSize);
    void enableMulticastLoopbackOption();
    void setMulticastTtlOption(int ttlValue);

#if 0
    void setProtocol(QCoapProtocol *protocol);
#endif

Q_SIGNALS:
    void finished(QCoapReply *);

protected:
    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
