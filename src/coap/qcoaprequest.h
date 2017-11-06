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

#ifndef QCOAPREQUEST_H
#define QCOAPREQUEST_H

#include <QtCoap/qcoapglobal.h>
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
    enum Operation {
        Empty,
        Get,
        Post,
        Put,
        Delete,
        Other
    };

    explicit QCoapRequest(const QUrl &url = QUrl(),
                 MessageType type = NonConfirmable,
                 const QUrl &proxyUrl = QUrl());
    QCoapRequest(const QCoapRequest &other,
                 QCoapRequest::Operation op = Empty);
    ~QCoapRequest() {}

    QCoapRequest &operator=(const QCoapRequest &other);
    bool operator<(const QCoapRequest &other) const;

    QUrl url() const;
    QUrl proxyUrl() const;
    Operation operation() const;
    bool observe() const;
    void setUrl(const QUrl &url);
    void setProxyUrl(const QUrl &proxyUrl);
    void setOperation(Operation operation);
    void enableObserve();

private:
    QCoapRequestPrivate* d_func();
    inline const QCoapRequestPrivate* d_func() const
    {
        return reinterpret_cast<const QCoapRequestPrivate*>(d_ptr.constData());
    }
};

Q_DECLARE_METATYPE(QCoapRequest::Operation)

QT_END_NAMESPACE

#endif // QCOAPREQUEST_H
