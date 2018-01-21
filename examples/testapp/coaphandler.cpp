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

#include "coaphandler.h"

#include <QDebug>
#include <QCoapClient>
#include <QCoapReply>
#include <QCoapDiscoveryReply>
#include <QtNetwork/QHostInfo>

CoapHandler::CoapHandler(const QHostAddress& coapHost, QObject *parent) : QObject(parent)
{
    qDebug() << "Started...";

    connect(&m_coapClient, &QCoapClient::error, this, &CoapHandler::onError);

    QCoapReply *getReply = m_coapClient.get(QUrl("coap://" + coapHost.toString() + "/test"));
    connect(getReply, &QCoapReply::finished, this, &CoapHandler::onFinished);

    QCoapReply *observeReply = m_coapClient.observe(QUrl("coap://" + coapHost.toString() + "/obs"));
    connect(observeReply, &QCoapReply::notified, this, &CoapHandler::onNotified);
    connect(observeReply, &QCoapReply::finished, this, &CoapHandler::onFinished);

    QCoapDiscoveryReply *discoveryReply = m_coapClient.discover(QUrl("coap://" + coapHost.toString()));
    connect(discoveryReply, &QCoapDiscoveryReply::discovered, this, &CoapHandler::onDiscovered);
    connect(discoveryReply, &QCoapReply::finished, this, &CoapHandler::onFinished);
}

void CoapHandler::onFinished(QCoapReply *reply)
{
    if (reply->errorReceived() == QtCoap::NoError)
        qDebug() << "Reply finished with payload: " << reply->readAll();
    else
        qDebug() << "Reply failed";

    // Don't forget to remove the reply
    reply->deleteLater();
}

void CoapHandler::onNotified(QCoapReply *reply, QCoapMessage message)
{
    Q_UNUSED(message)

    // You can alternatively use `message.payload();`
    qDebug() << "Received OBSERVE notification: " << reply->readAll();
}

void CoapHandler::onDiscovered(QCoapDiscoveryReply *reply, QVector<QCoapResource> resources)
{
    Q_UNUSED(reply)

    for (const QCoapResource &res : qAsConst(resources))
        qDebug() << "Resource discovered: " << res.path() << res.title();
}

void CoapHandler::onError(QCoapReply *reply, QtCoap::Error error)
{
    if (reply)
        qDebug() << "CoAP reply error: " << reply->errorString();
    else
        qDebug() << "CoAP error: " << error;
}
