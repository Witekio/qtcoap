/****************************************************************************
**
** Copyright (C) 2018 Witekio.
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

#ifndef QCOAPPARSER_H
#define QCOAPPARSER_H

#include <QtCore/qglobal.h>
#include "qcoapnamespace.h"
#include "qcoapmessage.h"

QT_BEGIN_NAMESPACE

class QCoapInternalMessage;

class Q_AUTOTEST_EXPORT QCoapParser
{
public:
    explicit QCoapParser() = default;

    QByteArray encode(const QCoapMessage &message, QtCoap::Method method) const;
    std::pair<QCoapMessage, QtCoap::ResponseCode> decode(const QByteArray &frame) const;
};

QT_END_NAMESPACE

#endif // QCOAPPARSER_H
