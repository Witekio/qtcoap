/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL3$
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

#ifndef QCOAPREQUEST_P_H
#define QCOAPREQUEST_P_H

#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoaprequest.h>
#include <private/qcoapmessage_p.h>

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

class Q_AUTOTEST_EXPORT QCoapRequestPrivate : public QCoapMessagePrivate
{
public:
    QCoapRequestPrivate(const QUrl &url = QUrl(),
            QCoapMessage::MessageType type = QCoapMessage::NonConfirmable,
            const QUrl &proxyUrl = QUrl());
    QCoapRequestPrivate(const QCoapRequestPrivate &other) = default;
    ~QCoapRequestPrivate();

    QUrl uri;
    QUrl proxyUri;
    QtCoap::Operation operation = QtCoap::Empty;
    bool observe = false;
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_P_H
