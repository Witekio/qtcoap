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

#ifndef QCOAPDISCOVERYREPLY_H
#define QCOAPDISCOVERYREPLY_H

#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapresource.h>
#include <QtCoap/qcoapprotocol.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class QCoapDiscoveryReplyPrivate;
class Q_COAP_EXPORT QCoapDiscoveryReply : public QCoapReply
{
public:
    explicit QCoapDiscoveryReply(QObject *parent = nullptr);

    QList<QCoapResource> resourceList() const;

private:
    void updateFromInternalReply(const QCoapInternalReply &internalReply) Q_DECL_OVERRIDE;
    Q_DECLARE_PRIVATE(QCoapDiscoveryReply)
};

QT_END_NAMESPACE

#endif // QCOAPDISCOVERYREPLY_H
