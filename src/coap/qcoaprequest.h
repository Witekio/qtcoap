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

#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoapmessage.h>
#include <QtCoap/qcoapconnection.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QCoapInternalRequest;
class QCoapRequestPrivate;
class Q_COAP_EXPORT QCoapRequest : public QCoapMessage
{
public:
    explicit QCoapRequest(const QUrl &url = QUrl(),
                 MessageType type = NonConfirmable,
                 const QUrl &proxyUrl = QUrl());
    explicit QCoapRequest(const char* url,
                 MessageType type = NonConfirmable);
    QCoapRequest(const QCoapRequest &other,
                 QtCoap::Method method = QtCoap::Invalid);
    ~QCoapRequest();

    QCoapRequest &operator=(const QCoapRequest &other);

    QUrl url() const;
    QUrl proxyUrl() const;
    QtCoap::Method method() const;
    bool isObserve() const;
    void setUrl(const QUrl &url);
    void setProxyUrl(const QUrl &proxyUrl);
    void setMethod(QtCoap::Method method);
    void enableObserve();

    bool isValid() const;
    static bool isUrlValid(const QUrl &url);

private:
    //! For QSharedDataPointer
    inline QCoapRequestPrivate* d_func();
    const QCoapRequestPrivate* d_func() const
    {
        return reinterpret_cast<const QCoapRequestPrivate*>(d_ptr.constData());
    }
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
