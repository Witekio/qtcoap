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

#include <QtCore/qstring.h>
#include <QtNetwork/qhostinfo.h>

/*!
    \internal

    This namespace provides URL and settings used in QtCoap tests.

    Tests require a Californium plugtest server, accessible with
    "coap-plugtest-server" host name.
*/
namespace QtCoapNetworkSettings {
    QString testServerHostName() {
        return QStringLiteral("coap-plugtest-server");
    }
    QString testServerHost() {
        QHostInfo host = QHostInfo::fromName(testServerHostName());
        if (host.addresses().isEmpty()) {
            QString error = "Host name " + testServerHostName() + " could not be resolved.";
            QWARN(qPrintable(error));
            return QString();
        }

        return host.addresses().first().toString();
    }
    QString testServerUrl() {
        return QStringLiteral("coap://") + testServerHost() + QStringLiteral(":5683");
    }
    QString testServerResource() {
        return testServerUrl() + QStringLiteral("/test");
    }
}
