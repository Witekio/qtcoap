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

#include <QtCoap/qcoapclient.h>
#include <QtCoap/qcoaprequest.h>
#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapdiscoveryreply.h>
#include <QtCore/qbuffer.h>
#include <QtNetwork/qnetworkdatagram.h>
#include <private/qcoapclient_p.h>
#include <private/qcoapconnection_p.h>

#include "../coapnetworksettings.h"

using namespace QtCoapNetworkSettings;

class tst_QCoapClient : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void incorrectUrls_data();
    void incorrectUrls();
    void methods_data();
    void methods();
    void separateMethod();
    void socketError();
    void timeout_data();
    void timeout();
    void abort();
    void removeReply();
    void setBlockSize_data();
    void setBlockSize();
    void requestWithQIODevice_data();
    void requestWithQIODevice();
    void multipleRequests();
    void blockwiseReply_data();
    void blockwiseReply();
    void blockwiseRequest_data();
    void blockwiseRequest();
    void discover_data();
    void discover();
    void observe_data();
    void observe();
};

class QCoapConnectionSocketTestsPrivate : public QCoapConnectionPrivate
{
    bool bind()
    {
        // Force a socket binding error
        QUdpSocket anotherSocket;
        anotherSocket.bind(QHostAddress::Any, 6080);
        return socket()->bind(QHostAddress::Any, 6080);
    }
};

class QCoapConnectionSocketTests : public QCoapConnection
{
public:
    QCoapConnectionSocketTests() :
        QCoapConnection(*new QCoapConnectionSocketTestsPrivate)
    {
        createSocket();
    }

private:
    Q_DECLARE_PRIVATE(QCoapConnectionSocketTests)
};

class QCoapClientForSocketErrorTests : public QCoapClient
{
public:
    QCoapClientForSocketErrorTests() :
        QCoapClient(new QCoapProtocol, new QCoapConnectionSocketTests)
    {}

    QCoapConnection *connection()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return privateClient->connection;
    }
};

class QCoapClientForTests : public QCoapClient
{
public:
    QCoapClientForTests() {}
    QCoapClientForTests(QCoapProtocol *protocol, QCoapConnection *connection) :
        QCoapClient(protocol, connection)
    {}

    QCoapProtocol *protocol()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return privateClient->protocol;
    }
    QCoapConnection *connection()
    {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate *>(d_func());
        return privateClient->connection;
    }
};

class Helper : public QObject
{
    Q_OBJECT
public:
    Helper() {}

public slots:
    void onError(QCoapReply *, QtCoap::Error error)
    {
        qWarning() << "Network error" << error << "occurred";
    }
};

void tst_QCoapClient::incorrectUrls_data()
{
    QWARN("Expect warnings here...");
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get")        << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("post")       << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("put")        << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("delete")     << QUrl("wrong://10.20.30.40:5683/test");
    QTest::newRow("discover")   << QUrl("wrong://10.20.30.40:5683/test");
}

void tst_QCoapClient::incorrectUrls()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QScopedPointer<QCoapReply> reply;

    if (qstrcmp(QTest::currentDataTag(), "get") == 0)
        reply.reset(client.get(url));
    else if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply.reset(client.post(url));
    else if (qstrcmp(QTest::currentDataTag(), "put") == 0)
        reply.reset(client.put(url));
    else if (qstrcmp(QTest::currentDataTag(), "delete") == 0)
        reply.reset(client.deleteResource(url));
    else if (qstrcmp(QTest::currentDataTag(), "discover") == 0)
        reply.reset(client.discover(url));
    else {
        QString error = QLatin1Literal("Unrecognized method '") + QTest::currentDataTag() + "'";
        QFAIL(qPrintable(error));
    }

    QVERIFY2(reply.isNull(), "Request did not fail as expected.");
}

void tst_QCoapClient::methods_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QtCoap::Method>("method");

    QTest::newRow("get_no_op")              << QUrl(testServerResource()) << QtCoap::Invalid;
    QTest::newRow("get")                    << QUrl(testServerResource()) << QtCoap::Get;
    QTest::newRow("get_incorrect_op")       << QUrl(testServerResource()) << QtCoap::Put;
    QTest::newRow("get_no_port")
                << QUrl("coap://" + testServerHost() + "/test") << QtCoap::Get;
    QTest::newRow("get_no_scheme_no_port")  << QUrl(testServerHost() + "/test") << QtCoap::Get;
    QTest::newRow("post_no_op")             << QUrl(testServerResource()) << QtCoap::Invalid;
    QTest::newRow("post")                   << QUrl(testServerResource()) << QtCoap::Post;
    QTest::newRow("post_incorrect_op")      << QUrl(testServerResource()) << QtCoap::Delete;
    QTest::newRow("put_no_op")              << QUrl(testServerResource()) << QtCoap::Invalid;
    QTest::newRow("put")                    << QUrl(testServerResource()) << QtCoap::Put;
    QTest::newRow("put_incorrect_op")       << QUrl(testServerResource()) << QtCoap::Post;
    QTest::newRow("delete_no_op")           << QUrl(testServerResource()) << QtCoap::Invalid;
    QTest::newRow("delete")                 << QUrl(testServerResource()) << QtCoap::Delete;
    QTest::newRow("delete_incorrect_op")    << QUrl(testServerResource()) << QtCoap::Get;
}

void tst_QCoapClient::methods()
{
    QFETCH(QUrl, url);
    QFETCH(QtCoap::Method, method);

    QCoapClient client;

    QCoapRequest request(url);
    if (method != QtCoap::Invalid)
        request.setMethod(method);

    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply *)));

    QScopedPointer<QCoapReply> reply;
    if (qstrncmp(QTest::currentDataTag(), "get", 3) == 0)
        reply.reset(client.get(request));
    else if (qstrncmp(QTest::currentDataTag(), "post", 4) == 0)
        reply.reset(client.post(request));
    else if (qstrncmp(QTest::currentDataTag(), "put", 3) == 0)
        reply.reset(client.put(request));
    else if (qstrncmp(QTest::currentDataTag(), "delete", 6) == 0)
        reply.reset(client.deleteResource(request));
    else {
        QString error = QLatin1Literal("Unrecognized method '") + QTest::currentDataTag() + "'";
        QFAIL(qPrintable(error));
    }

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished(QCoapReply *)));
    QTRY_COMPARE(spyReplyFinished.count(), 1);
    QTRY_COMPARE(spyClientFinished.count(), 1);

    QByteArray replyData;
    if (!reply.isNull()) {
        replyData = reply->readAll();
        if (qstrncmp(QTest::currentDataTag(), "get", 3) == 0) {
            QVERIFY(!replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::Content);
        } else if (qstrncmp(QTest::currentDataTag(), "post", 4) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::Created);
        } else if (qstrncmp(QTest::currentDataTag(), "put", 3) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::Changed);
        } else if (qstrncmp(QTest::currentDataTag(), "delete", 6) == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->responseCode(), QtCoap::Deleted);
        } else {
            QString error = QLatin1Literal("Unrecognized method '") + QTest::currentDataTag() + "'";
            QFAIL(qPrintable(error));
        }
    }
}

void tst_QCoapClient::separateMethod()
{
    QCoapClient client;
    QScopedPointer<QCoapReply> reply(client.get(QUrl(testServerUrl() + "/separate")));

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished(QCoapReply *)));
    QTRY_COMPARE(spyReplyFinished.count(), 1);

    QByteArray replyData = reply->readAll();

    QVERIFY(!replyData.isEmpty());
    QCOMPARE(reply->responseCode(), QtCoap::Content);
}

void tst_QCoapClient::removeReply()
{
    QCoapClient client;
    QCoapReply *reply = client.get(QUrl(testServerResource()));
    QVERIFY2(reply != nullptr, "Request failed unexpectedly");

    try {
        reply->deleteLater();

        QEventLoop eventLoop;
        QTimer::singleShot(2000, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();
    } catch (...) {
        QFAIL("Exception occurred after destroying the QCoapReply");
    }
}

void tst_QCoapClient::setBlockSize_data()
{
    QTest::addColumn<int>("blockSizeSet");
    QTest::addColumn<int>("blockSizeExpected");

    QTest::newRow("valid_size_0")       << 0 << 0;
    QTest::newRow("valid_size_16")      << 16 << 16;
    QTest::newRow("valid_size_1024")    << 1024 << 1024;
    QTest::newRow("invalid_size_8")     << 8 << 0;
    QTest::newRow("invalid_size_350")   << 350 << 0;
    QTest::newRow("invalid_size_2048")  << 2048 << 0;
}

void tst_QCoapClient::setBlockSize()
{
    QFETCH(int, blockSizeSet);
    QFETCH(int, blockSizeExpected);

    QCoapClientForTests client;
    client.setBlockSize(blockSizeSet);

    QEventLoop eventLoop;
    QTimer::singleShot(1000, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QCOMPARE(client.protocol()->blockSize(), blockSizeExpected);
}

void tst_QCoapClient::requestWithQIODevice_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("post") << QUrl(testServerResource());
    QTest::newRow("put") << QUrl(testServerResource());
}

void tst_QCoapClient::requestWithQIODevice()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QCoapRequest request(url);

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    buffer.write("Some data");

    QScopedPointer<QCoapReply> reply;
    if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply.reset(client.post(request, &buffer));
    else
        reply.reset(client.put(request, &buffer));

    QVERIFY2(!reply.isNull(), "Request failed unexpectedly");
    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished(QCoapReply *)));
    QTRY_COMPARE(spyReplyFinished.count(), 1);

    QByteArray replyData = reply->readAll();

    if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->responseCode(), QtCoap::Created);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->responseCode(), QtCoap::Changed);
    }
}

void tst_QCoapClient::multipleRequests()
{
    QCoapClient client;
    QUrl url = QUrl(testServerResource());
    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply *)));

    QScopedPointer<QCoapReply> replyGet1(client.get(url));
    QScopedPointer<QCoapReply> replyGet2(client.get(url));
    QScopedPointer<QCoapReply> replyGet3(client.get(url));
    QScopedPointer<QCoapReply> replyGet4(client.get(url));

    QVERIFY2(!replyGet1.isNull(), "Request failed unexpectedly");
    QVERIFY2(!replyGet2.isNull(), "Request failed unexpectedly");
    QVERIFY2(!replyGet3.isNull(), "Request failed unexpectedly");
    QVERIFY2(!replyGet4.isNull(), "Request failed unexpectedly");

    QSignalSpy spyReplyGet1Finished(replyGet1.data(), SIGNAL(finished(QCoapReply *)));
    QSignalSpy spyReplyGet2Finished(replyGet2.data(), SIGNAL(finished(QCoapReply *)));
    QSignalSpy spyReplyGet3Finished(replyGet3.data(), SIGNAL(finished(QCoapReply *)));
    QSignalSpy spyReplyGet4Finished(replyGet4.data(), SIGNAL(finished(QCoapReply *)));

    QTRY_COMPARE(spyReplyGet1Finished.count(), 1);
    QTRY_COMPARE(spyReplyGet2Finished.count(), 1);
    QTRY_COMPARE(spyReplyGet3Finished.count(), 1);
    QTRY_COMPARE(spyReplyGet4Finished.count(), 1);
    QTRY_COMPARE(spyClientFinished.count(), 4);

    QByteArray replyData1 = replyGet1->readAll();
    QByteArray replyData2 = replyGet2->readAll();
    QByteArray replyData3 = replyGet3->readAll();
    QByteArray replyData4 = replyGet4->readAll();

    QCOMPARE(replyGet1->responseCode(), QtCoap::Content);
    QCOMPARE(replyGet2->responseCode(), QtCoap::Content);
    QCOMPARE(replyGet3->responseCode(), QtCoap::Content);
    QCOMPARE(replyGet4->responseCode(), QtCoap::Content);

    QVERIFY(replyData1 != replyData2);
    QVERIFY(replyData1 != replyData3);
    QVERIFY(replyData1 != replyData4);
    QVERIFY(replyData2 != replyData3);
    QVERIFY(replyData2 != replyData4);
    QVERIFY(replyData3 != replyData4);
}

void tst_QCoapClient::socketError()
{
    QCoapClientForSocketErrorTests client;
    QUrl url = QUrl(testServerResource());

    QUdpSocket *socket = client.connection()->socket();
    QVERIFY2(socket, "Socket not properly created with connection");
    QSignalSpy spySocketError(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QScopedPointer<QCoapReply> reply(client.get(url));
    QSignalSpy spyClientError(&client, &QCoapClient::error);

    QTRY_COMPARE_WITH_TIMEOUT(spySocketError.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientError.count(), 1, 1000);
    QCOMPARE(spyClientError.first().at(1), QtCoap::AddressInUseError);
}
void tst_QCoapClient::timeout_data()
{
    QTest::addColumn<int>("timeout");
    QTest::addColumn<int>("maxRetransmit");

    QTest::newRow("2000/0") << 2000 << 0;
    QTest::newRow("2000/2") << 2000 << 2;
    QTest::newRow("4000/0") << 4000 << 0;
}

void tst_QCoapClient::timeout()
{
    QFETCH(int, timeout);
    QFETCH(int, maxRetransmit);

    QCoapClientForTests client;
    // Trigger a network timeout
    client.protocol()->setAckTimeout(timeout);
    client.protocol()->setAckRandomFactor(1);
    client.protocol()->setMaxRetransmit(maxRetransmit);
    QUrl url = QUrl("coap://240.0.0.0:5683/"); // Need an url that returns nothing

    QElapsedTimer timeoutTimer;
    timeoutTimer.start();
    QScopedPointer<QCoapReply> reply(client.get(QCoapRequest(url, QCoapMessage::Confirmable)));
    QSignalSpy spyClientError(&client, &QCoapClient::error);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);
    QSignalSpy spyReplyAborted(reply.data(), &QCoapReply::aborted);
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    // Check timeout upper limit
    int transmissions = maxRetransmit + 1;

    // 10% Precision expected at least, plus timer precision
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.count(), 1, static_cast<int>(
                                  1.1 * client.protocol()->maxTransmitWait() + 20 * transmissions));

    // Check timeout lower limit
    qint64 elapsedTime = timeoutTimer.elapsed();
    QString errorMessage = QString("Timeout was triggered after %1ms, while expecting about %2ms")
                           .arg(QString::number(elapsedTime),
                                QString::number(client.protocol()->maxTransmitWait()));

    // 10% Precision expected at least, minus timer precision
    QVERIFY2(elapsedTime > 0.9 * client.protocol()->maxTransmitWait() - 20 * transmissions,
             qPrintable(errorMessage));

    QCOMPARE(spyReplyError.first().at(1), QtCoap::TimeOutError);
    QCOMPARE(spyReplyFinished.count(), 1);
    QCOMPARE(spyReplyAborted.count(), 0);
    QCOMPARE(spyClientError.count(), 1);
}

void tst_QCoapClient::abort()
{
    QCoapClient client;
    QUrl url = QUrl(testServerUrl() + "/large");

    QScopedPointer<QCoapReply> reply(client.get(url));
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QSignalSpy spyReplyAborted(reply.data(), &QCoapReply::aborted);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);

    reply->abortRequest();

    QEventLoop eventLoop;
    QTimer::singleShot(1000, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    QCOMPARE(spyReplyAborted.count(), 1);
    QCOMPARE(spyReplyFinished.count(), 1);
    QCOMPARE(spyReplyError.count(), 0);
}

void tst_QCoapClient::blockwiseReply_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<QByteArray>("replyData");

    QByteArray data;
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 1 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 2 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 3 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 4 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 5 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");

    QTest::newRow("get_large")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::NonConfirmable
            << data;
    QTest::newRow("get_large_separate")
            << QUrl(testServerUrl() + "/large-separate")
            << QCoapMessage::NonConfirmable
            << data;
    QTest::newRow("get_large_confirmable")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Confirmable
            << data;
    QTest::newRow("get_large_separate_confirmable")
            << QUrl(testServerUrl() + "/large-separate")
            << QCoapMessage::Confirmable
            << data;
    QTest::newRow("get_large_16bits")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::NonConfirmable
            << data;
    QTest::newRow("get_large_16bits_confirmable")
            << QUrl(testServerUrl() + "/large")
            << QCoapMessage::Confirmable
            << data;
}

void tst_QCoapClient::blockwiseReply()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(QByteArray, replyData);

    QCoapClient client;
    QCoapRequest request(url);

    if (qstrncmp(QTest::currentDataTag(), "get_large_16bits", 16) == 0)
        client.setBlockSize(16);

    request.setType(type);
    QScopedPointer<QCoapReply> reply(client.get(request));
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);
    QSignalSpy spyReplyError(reply.data(), &QCoapReply::error);
    Helper helper;
    connect(reply.data(), &QCoapReply::error, &helper, &Helper::onError);

    QCOMPARE(spyReplyError.count(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);
    QCOMPARE(reply->readAll(), replyData);
}

void tst_QCoapClient::blockwiseRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<QByteArray>("requestData");
    QTest::addColumn<QtCoap::ResponseCode>("responseCode");
    QTest::addColumn<QByteArray>("replyData");

    QByteArray data;
    const char alphabet[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
    for (int i = 3; i-- > 0; )
        data.append(alphabet);

    QTest::newRow("large_post_empty_reply") << QUrl(testServerUrl() + "/query")
                                            << QCoapMessage::NonConfirmable
                                            << data
                                            << QtCoap::MethodNotAllowed
                                            << QByteArray();
    QTest::newRow("large_post_large_reply") << QUrl(testServerUrl() + "/large-post")
                                            << QCoapMessage::NonConfirmable
                                            << data
                                            << QtCoap::Changed
                                            << data.toUpper();
}

void tst_QCoapClient::blockwiseRequest()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(QByteArray, requestData);
    QFETCH(QtCoap::ResponseCode, responseCode);
    QFETCH(QByteArray, replyData);

    QCoapClient client;
    client.setBlockSize(16);

    QCoapRequest request(url);
    request.setType(type);
    request.addOption(QCoapOption::ContentFormat);

    QScopedPointer<QCoapReply> reply(client.post(request, requestData));
    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished(QCoapReply *)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);

    QByteArray dataReply = reply->readAll();
    QCOMPARE(dataReply, replyData);
    QCOMPARE(reply->responseCode(), responseCode);
}

void tst_QCoapClient::discover_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<int>("resourceNumber");

    // Californium test server exposes 29 resources
    QTest::newRow("discover") << QUrl(testServerUrl())
                              << 29;
}

void tst_QCoapClient::discover()
{
    QFETCH(QUrl, url);
    QFETCH(int, resourceNumber);

    QCoapClient client;

    QScopedPointer<QCoapDiscoveryReply> resourcesReply(client.discover(url)); // /.well-known/core
    QSignalSpy spyReplyFinished(resourcesReply.data(), SIGNAL(finished(QCoapReply *)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);

    QCOMPARE(resourcesReply->resources().length(), resourceNumber);

    //! TODO Test discovery content too
}

void tst_QCoapClient::observe_data()
{
    QWARN("Observe tests may take some time, don't forget to raise Tests timeout in settings.");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::MessageType>("type");

    QTest::newRow("observe")
            << QUrl(testServerUrl() + "/obs")
            << QCoapMessage::NonConfirmable;

    QTest::newRow("observe_confirmable")
            << QUrl(testServerUrl() + "/obs")
            << QCoapMessage::Confirmable;

    QTest::newRow("observe_receive")
            << QUrl(testServerUrl() + "/obs-non")
            << QCoapMessage::NonConfirmable;

    QTest::newRow("observe_receive_confirmable")
            << QUrl(testServerUrl() + "/obs-non")
            << QCoapMessage::Confirmable;

    QTest::newRow("observe_large")
            << QUrl(testServerUrl() + "/obs-large")
            << QCoapMessage::NonConfirmable;

    QTest::newRow("observe_large_confirmable")
            << QUrl(testServerUrl() + "/obs-large")
            << QCoapMessage::Confirmable;

    QTest::newRow("observe_pumping")
            << QUrl(testServerUrl() + "/obs-pumping")
            << QCoapMessage::NonConfirmable;

    QTest::newRow("observe_pumping_confirmable")
            << QUrl(testServerUrl() + "/obs-pumping")
            << QCoapMessage::Confirmable;
}

void tst_QCoapClient::observe()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::MessageType, type);

    QCoapClient client;
    QCoapRequest request(url);

    request.setType(type);
    QSharedPointer<QCoapReply> reply(client.observe(request),
                                     &QObject::deleteLater);
    QSignalSpy spyReplyNotified(reply.data(), &QCoapReply::notified);
    QSignalSpy spyReplyFinished(reply.data(), &QCoapReply::finished);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.count(), 3, 30000);
    client.cancelObserve(reply.data());

    QVERIFY2(!spyReplyNotified.wait(7000), "'Notify' signal received after cancelling observe");
    QCOMPARE(spyReplyFinished.count(), 1);

    for (QList<QVariant> receivedSignals : qAsConst(spyReplyNotified)) {
        QRegularExpression regexp(QStringLiteral("..:..:.."));
        QByteArray payload = receivedSignals.at(1).value<QCoapMessage>().payload();
        QString error = QString("Invalid payload for 'notified' signal: %1").arg(QString(payload));
        QVERIFY2(regexp.match(payload).hasMatch(), qPrintable(error));
    }
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
