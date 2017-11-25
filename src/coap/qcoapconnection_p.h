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

#ifndef QCOAPCONNECTION_P_H
#define QCOAPCONNECTION_P_H

#include <QtCoap/qcoapconnection.h>
#include <QtNetwork/qudpsocket.h>
#include <QtCore/qqueue.h>
#include <private/qobject_p.h>

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

struct CoapFrame {
    QByteArray currentPdu;
    QString host;
    quint16 port = 0;

    CoapFrame(const QByteArray &pdu, const QString &hostName, quint16 portNumber)
    : currentPdu(pdu), host(hostName), port(portNumber) {}
};

class Q_AUTOTEST_EXPORT QCoapConnectionPrivate : public QObjectPrivate
{
public:
    QCoapConnectionPrivate() {}

    QCoapConnection::ConnectionState state = QCoapConnection::Unconnected;
    QQueue<CoapFrame> framesToSend;

    virtual bool bind();

    void bindSocket();
    void writeToSocket(const CoapFrame &frame);
    QUdpSocket* socket() { return udpSocket; }
    void setSocket(QUdpSocket *socket);
    void setState(QCoapConnection::ConnectionState newState);

    void _q_socketBound();
    void _q_socketReadyRead();
    void _q_startToSendRequest();
    void _q_socketError(QAbstractSocket::SocketError);

private:
    QUdpSocket *udpSocket = nullptr;

    Q_DECLARE_PUBLIC(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_P_H
