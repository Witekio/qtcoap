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

#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qudpsocket.h>

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate;
class Q_COAP_EXPORT QCoapConnection : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Unconnected,
        Bound
    };

    explicit QCoapConnection(QObject *parent = nullptr);

    void sendRequest(const QByteArray &request, const QString &host, quint16 port);

    QUdpSocket *socket() const;
    ConnectionState state() const;

Q_SIGNALS:
    void bound();
    void error(QAbstractSocket::SocketError);
    void readyRead(const QByteArray &frame);

protected:
    explicit QCoapConnection(QCoapConnectionPrivate &dd, QObject *parent = nullptr);

    virtual void createSocket();

    Q_DECLARE_PRIVATE(QCoapConnection)
    Q_PRIVATE_SLOT(d_func(), void _q_socketReadyRead())
    Q_PRIVATE_SLOT(d_func(), void _q_socketBound())
    Q_PRIVATE_SLOT(d_func(), void _q_startToSendRequest())
    Q_PRIVATE_SLOT(d_func(), void _q_socketError(QAbstractSocket::SocketError))
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
