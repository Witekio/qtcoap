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

#include <QtTest>
#include <QCoreApplication>

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCore/qbuffer.h>
#include <QtNetwork/qudpsocket.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapconnection.h>
#include <QtCoap/qcoaprequest.h>
#include <private/qcoapconnection_p.h>
#include <private/qcoapinternalrequest_p.h>

class tst_QCoapConnection : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void ctor();
    void connectToHost();
    void sendRequest_data();
    void sendRequest();
};

class QCoapConnectionForTest : public QCoapConnection
{
    Q_OBJECT
public:
    QCoapConnectionForTest(QObject *parent = nullptr) :
        QCoapConnection(parent)
    {}

    void bindSocketForTest() { d_func()->bindSocket(); }
};

void tst_QCoapConnection::ctor()
{
    QCoapConnection connection;
    QVERIFY(connection.socket());
}

void tst_QCoapConnection::connectToHost()
{
    QCoapConnectionForTest connection;

    QUdpSocket *socket = qobject_cast<QUdpSocket*>(connection.socket());
    QSignalSpy spyConnectionBound(&connection, SIGNAL(bound()));
    QSignalSpy spySocketStateChanged(socket , SIGNAL(stateChanged(QAbstractSocket::SocketState)));

    QCOMPARE(connection.state(), QCoapConnection::Unconnected);

    connection.bindSocketForTest();

    QTRY_COMPARE_WITH_TIMEOUT(spySocketStateChanged.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionBound.count(), 1, 5000);
    QCOMPARE(connection.state(), QCoapConnection::Bound);
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<quint16>("port");
    QTest::addColumn<QtCoap::Method>("method");
    QTest::addColumn<QString>("dataHexaHeader");
    QTest::addColumn<QString>("dataHexaPayload");

    QTest::newRow("simple_get_request")
        << "coap://"
        << "172.17.0.3" << "/test" << quint16(5683)
        << QtCoap::Get << "5445"
        << "61626364c0211eff547970653a203120284e4f4e290a436f64653a2031202847"
           "4554290a4d49443a2032343830360a546f6b656e3a203631363236333634";

    QTest::newRow("simple_put_request")
        << "coap://"
        << "172.17.0.3"
        << "/test"
        << quint16(5683)
        << QtCoap::Put
        << "5444"
        << "61626364";

    QTest::newRow("simple_post_request")
        << "coap://"
        << "172.17.0.3"
        << "/test"
        << quint16(5683)
        << QtCoap::Post
        << "5441"
        << "61626364896c6f636174696f6e31096c6f636174696f6e32096c6f636174696f"
           "6e33";

    QTest::newRow("simple_delete_request")
        << "coap://"
        << "172.17.0.3"
        << "/test"
        << quint16(5683)
        << QtCoap::Delete
        << "5442"
        << "61626364";
}

void tst_QCoapConnection::sendRequest()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(quint16, port);
    QFETCH(QtCoap::Method, method);
    QFETCH(QString, dataHexaHeader);
    QFETCH(QString, dataHexaPayload);

    QCoapConnectionForTest connection;

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(&connection, SIGNAL(readyRead(const QByteArray&)));

    QCoapRequest request(protocol + host + path);
    request.setMessageId(24806);
    request.setToken(QByteArray("abcd"));
    request.setMethod(method);
    QVERIFY(connection.socket() != nullptr);
    QCoapInternalRequest internalRequest(request);
    connection.sendRequest(internalRequest.toQByteArray(), host, port);

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 5000);

    QByteArray reply = spyConnectionReadyRead.first().first().toByteArray();

    QVERIFY(QString(reply.toHex()).startsWith(dataHexaHeader));
    QVERIFY(QString(reply.toHex()).endsWith(dataHexaPayload));
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
