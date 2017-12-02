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

#include "qcoapdiscoveryreply_p.h"
#include "qcoapinternalreply_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCoapDiscoveryReply
    \brief A QCoapDiscoveryReply object is a QCoapReply that stores also a
    list of QCoapResources.

    \reentrant

    This class is used for discovery requests.

    \sa QCoapClient, QCoapRequest, QCoapReply, QCoapResource
*/

/*!
    Constructs a new QCoapDiscoveryReply and sets \a parent as parent object.
*/
QCoapDiscoveryReply::QCoapDiscoveryReply(QObject *parent) :
    QCoapReply (* new QCoapDiscoveryReplyPrivate, parent)
{

}

/*!
    Returns the list of resources.
*/
QList<QCoapResource> QCoapDiscoveryReply::resourceList() const
{
    Q_D(const QCoapDiscoveryReply);
    return d->resources;
}

/*!
    \internal

    Updates the QCoapDiscoveryReply object, its message and list of resources
    with data of the internal reply \a internalReply.
*/
void QCoapDiscoveryReply::updateFromInternalReply(const QCoapInternalReply &internalReply)
{
    Q_D(QCoapDiscoveryReply);

    if (!d->isAborted) {
        const QCoapMessage *internalReplyMessage = internalReply.message();

        d->message.setPayload(internalReplyMessage->payload());
        d->message.setType(internalReplyMessage->type());
        d->message.setVersion(internalReplyMessage->version());
        d->status = internalReply.statusCode();

        d->isFinished = true;
        d->isRunning = false;

        if (d->status >= QtCoap::BadRequest)
            replyError(d->status);
        else
            d->resources = QCoapProtocol::resourcesFromCoreLinkList(internalReplyMessage->payload());

        emit finished();
    }
}

QT_END_NAMESPACE
