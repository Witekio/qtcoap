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

#ifndef QCOAPCLIENT_H
#define QCOAPCLIENT_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCore/qobject.h>
#include <QtCore/qiodevice.h>
#include <QtNetwork/qabstractsocket.h>

QT_BEGIN_NAMESPACE

class QCoapReply;
class QCoapDiscoveryReply;
class QCoapRequest;
class QCoapProtocol;
class QCoapConnection;

class QCoapClientPrivate;
class Q_COAP_EXPORT QCoapClient : public QObject
{
    Q_OBJECT
public:
    explicit QCoapClient(QObject *parent = nullptr);
    ~QCoapClient();

    QCoapReply *get(const QCoapRequest &request);
    QCoapReply *get(const QUrl &url);
    QCoapReply *put(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *put(const QCoapRequest &request, QIODevice *device);
    QCoapReply *put(const QUrl &url, const QByteArray &data = QByteArray());
    QCoapReply *post(const QCoapRequest &request, const QByteArray &data = QByteArray());
    QCoapReply *post(const QCoapRequest &request, QIODevice *device);
    QCoapReply *post(const QUrl &url, const QByteArray &data = QByteArray());
    QCoapReply *deleteResource(const QCoapRequest &request);
    QCoapReply *deleteResource(const QUrl &url);
    QCoapReply *observe(const QCoapRequest &request);
    QCoapReply *observe(const QUrl &request);
    void cancelObserve(QCoapReply *notifiedReply);

#if 0
    //! TODO Add Multicast discovery in a later submission.
    QCoapDiscoveryReply *discover(const QString &discoveryPath = QLatin1String("/.well-known/core"));
#endif
    QCoapDiscoveryReply *discover(const QUrl &baseUrl,
                                  const QString &discoveryPath = QLatin1String("/.well-known/core"));

    void setBlockSize(quint16 blockSize);
    void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);

#if 0
    void setProtocol(QCoapProtocol *protocol);
#endif

Q_SIGNALS:
    void finished(QCoapReply *reply);
    void error(QCoapReply *reply, QtCoap::Error error);

protected:
    explicit QCoapClient(QCoapProtocol *protocol, QCoapConnection *connection,
                         QObject *parent = nullptr);

    Q_DECLARE_PRIVATE(QCoapClient)
};

QT_END_NAMESPACE

#endif // QCOAPCLIENT_H
