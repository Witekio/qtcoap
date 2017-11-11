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

#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include <QtCoap/qcoapmessage.h>
#include <QtCoap/qcoaprequest.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

class QCoapInternalReply;
class QCoapReplyPrivate;
class Q_COAP_EXPORT QCoapReply : public QIODevice
{
    Q_OBJECT
public:
    enum NetworkError {
        NoError,
        HostNotFoundError,
        BadRequestError,
        AddressInUseError,
        TimeOutError,
        UnauthorizedError,
        BadOptionError,
        ForbiddenError,
        NotFoundError,
        MethodNotAllowedError,
        NotAcceptableError,
        RequestEntityIncompleteError,
        PreconditionFailedError,
        RequestEntityTooLargeError,
        UnsupportedContentFormatError,
        InternalServerErrorError,
        NotImplementedError,
        BadGatewayError,
        ServiceUnavailableError,
        GatewayTimeoutError,
        ProxyingNotSupportedError,
        UnknownError
    };
    Q_ENUM(NetworkError)

    explicit QCoapReply(QObject *parent = nullptr);
    ~QCoapReply();

    QtCoap::StatusCode statusCode() const;
    QCoapMessage message() const;
    QCoapRequest request() const;
    QUrl url() const;
    QCoapRequest::Operation operation() const;
    NetworkError errorReceived() const;
    bool isRunning() const;
    bool isFinished() const;
    bool isAborted() const;
    void setRequest(const QCoapRequest &request);

Q_SIGNALS:
    void finished();
    void notified(const QByteArray &payload);
    void error(QCoapReply::NetworkError error);
    void aborted(QCoapReply*);

protected Q_SLOTS:
    void connectionError(QAbstractSocket::SocketError error);
    void replyError(QtCoap::StatusCode statusCode);

public Q_SLOTS:
    void abortRequest();

protected:
    friend class QCoapProtocol;
    friend class QCoapProtocolPrivate;

    explicit QCoapReply(QCoapReplyPrivate &dd, QObject *parent = nullptr);

    void setIsRunning(bool isRunning);
    void setError(NetworkError error);
    virtual void updateFromInternalReply(const QCoapInternalReply &internalReply);
    qint64 readData(char *data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char *data, qint64 maxSize) Q_DECL_OVERRIDE;

    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_H
