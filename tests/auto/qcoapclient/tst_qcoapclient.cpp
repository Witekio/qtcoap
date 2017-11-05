#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapclient.h>
#include <QtCoap/qcoaprequest.h>
#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapdiscoveryreply.h>
#include <QtCore/qbuffer.h>
#include <private/qcoapclient_p.h>
#include <private/qcoapconnection_p.h>

class tst_QCoapClient : public QObject
{
    Q_OBJECT

public:
    tst_QCoapClient() {}

private Q_SLOTS:
    void operations_data();
    void operations();
    void separateOperation_data();
    void separateOperation();
    void socketError();
    void timeout();
    void abort();
    void removeReply_data();
    void removeReply();
    void requestWithQIODevice_data();
    void requestWithQIODevice();
    void multipleRequests_data();
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

class QCoapConnectionForSocketErrorTestsPrivate : public QCoapConnectionPrivate
{
    bool bind() {
        QUdpSocket anotherSocket;
        anotherSocket.bind(QHostAddress::Any, 6080);
        return udpSocket->bind(QHostAddress::Any, 6080);
    }
};

class QCoapConnectionForSocketErrorTests : public QCoapConnection
{
public:
    QCoapConnectionForSocketErrorTests() :
        QCoapConnection (* new QCoapConnectionForSocketErrorTestsPrivate)
    {}

private:
    Q_DECLARE_PRIVATE(QCoapConnectionForSocketErrorTests)
};

class QCoapClientForSocketErrorTests : public QCoapClient
{
public:
    QCoapClientForSocketErrorTests() {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate*>(d_func());
        privateClient->connection = new QCoapConnectionForSocketErrorTests;
        privateClient->connection->moveToThread(privateClient->workerThread);
    }

    QCoapConnection *connection() {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate*>(d_func());
        return privateClient->connection;
    }
};

class QCoapClientForTests : public QCoapClient
{
public:
    QCoapClientForTests() {}

    QCoapProtocol *protocol() {
        QCoapClientPrivate *privateClient = static_cast<QCoapClientPrivate*>(d_func());
        return privateClient->protocol;
    }

};

class Helper : public QObject
{
    Q_OBJECT
public:
    Helper() {}

public slots:
    void onError(QCoapReply::NetworkError error) {
        qWarning() << "Network error" << error << "occured";
    }
};

void tst_QCoapClient::operations_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("post") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("put") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("delete") << QUrl("coap://172.17.0.3:5683/test");
}

void tst_QCoapClient::operations()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QCoapRequest request(url);
    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply*)));

    QScopedPointer<QCoapReply> reply;
    if (qstrcmp(QTest::currentDataTag(), "get") == 0)
        reply.reset(client.get(request));
    else if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply.reset(client.post(request));
    else if (qstrcmp(QTest::currentDataTag(), "put") == 0)
        reply.reset(client.put(request));
    else if (qstrcmp(QTest::currentDataTag(), "delete") == 0)
        reply.reset(client.deleteResource(request));
    else {
        QString error = QLatin1Literal("Unrecognized operation '") + QTest::currentDataTag() + "'";
        QFAIL(qPrintable(error));
    }

    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 5000);

    QVERIFY(!reply.isNull());
    QByteArray replyData;
    if (!reply.isNull()) {
        replyData = reply->readAll();
        if (qstrcmp(QTest::currentDataTag(), "get") == 0) {
            QVERIFY(!replyData.isEmpty());
            QCOMPARE(reply->statusCode(), QCoapReply::Content);
        } else if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), QCoapReply::Created);
        } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), QCoapReply::Changed);
        } else if (qstrcmp(QTest::currentDataTag(), "delete") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), QCoapReply::Deleted);
        } else {
            QFAIL("Unrecognized operation");
        }
    }
}

void tst_QCoapClient::separateOperation_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/separate");
}

void tst_QCoapClient::separateOperation()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QCoapRequest request(url);

    QScopedPointer<QCoapReply> reply(client.get(request));

    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(!reply.isNull());
    QByteArray replyData = reply->readAll();

    QVERIFY(!replyData.isEmpty());
    QCOMPARE(reply->statusCode(), QCoapReply::Content);
}

void tst_QCoapClient::removeReply_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/test");
}

void tst_QCoapClient::removeReply()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply *reply = client.get(request);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));

    // User deletes the reply
    delete reply;
    reply = nullptr;

    QThread::sleep(2);
    QCOMPARE(spyReplyFinished.count(), 0);
}

void tst_QCoapClient::requestWithQIODevice_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("post") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("put") << QUrl("coap://172.17.0.3:5683/test");
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

    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(!reply.isNull());
    QByteArray replyData = reply->readAll();

    if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), QCoapReply::Created);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), QCoapReply::Changed);
    }
}

void tst_QCoapClient::multipleRequests_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("request") << QUrl("coap://172.17.0.3:5683/test");
}

void tst_QCoapClient::multipleRequests()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply*)));

    QScopedPointer<QCoapReply> replyGet1(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyGet1Finished(replyGet1.data(), SIGNAL(finished()));
    QScopedPointer<QCoapReply> replyGet2(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyGet2Finished(replyGet2.data(), SIGNAL(finished()));
    QScopedPointer<QCoapReply> replyGet3(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyGet3Finished(replyGet3.data(), SIGNAL(finished()));
    QScopedPointer<QCoapReply> replyGet4(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyGet4Finished(replyGet4.data(), SIGNAL(finished()));

    QVERIFY(!replyGet1.isNull());
    QVERIFY(!replyGet2.isNull());
    QVERIFY(!replyGet3.isNull());
    QVERIFY(!replyGet4.isNull());

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet1Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet2Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet3Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet4Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 4, 5000);

    QByteArray replyData1 = replyGet1->readAll();
    QByteArray replyData2 = replyGet2->readAll();
    QByteArray replyData3 = replyGet3->readAll();
    QByteArray replyData4 = replyGet4->readAll();

    QCOMPARE(replyGet1->statusCode(), QCoapReply::Content);
    QCOMPARE(replyGet2->statusCode(), QCoapReply::Content);
    QCOMPARE(replyGet3->statusCode(), QCoapReply::Content);
    QCOMPARE(replyGet4->statusCode(), QCoapReply::Content);

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
    QUrl url = QUrl("coap://172.17.0.3:5683/test");

    QUdpSocket *socket = client.connection()->socket();
    QSignalSpy spySocketError(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QScopedPointer<QCoapReply> reply(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyError(reply.data(), SIGNAL(error(QCoapReply::NetworkError)));

    QTRY_COMPARE_WITH_TIMEOUT(spySocketError.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.count(), 1, 10000);
    QCOMPARE(spyReplyError.first().first(), QCoapReply::AddressInUseError);
}

void tst_QCoapClient::timeout()
{
    QWARN("Timeout test may take some times...");

    QCoapClientForTests client;
    client.protocol()->setAckTimeout(300);
    QUrl url = QUrl("coap://172.17.0.5:5683/"); // Need an url that return nothing

    QScopedPointer<QCoapReply> reply(client.get(QCoapRequest(url, QCoapMessage::Confirmable)));
    QSignalSpy spyReplyError(reply.data(), SIGNAL(error(QCoapReply::NetworkError)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.count(), 1, 60000);
    QCOMPARE(spyReplyError.first().first(), QCoapReply::TimeOutError);
}

void tst_QCoapClient::abort()
{
    QCoapClient client;
    QUrl url = QUrl("coap://172.17.0.3:5683/large");

    QScopedPointer<QCoapReply> reply(client.get(QCoapRequest(url)));
    QSignalSpy spyReplyGet1Finished(reply.data(), SIGNAL(finished()));
    reply->abortRequest();

    QThread::sleep(1);
    QCOMPARE(spyReplyGet1Finished.count(), 0);
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

    QTest::newRow("get_large") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::NonConfirmable
                               << data;
    QTest::newRow("get_large_separate") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::NonConfirmable
                               << data;
    QTest::newRow("get_large_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::Confirmable
                               << data;
    QTest::newRow("get_large_separate_confirmable") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::Confirmable
                               << data;
    QTest::newRow("get_large_16bits") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::NonConfirmable
                               << data;
    QTest::newRow("get_large_16bits_confirmable") << QUrl("coap://172.17.0.3:5683/large")
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

    if (qstrcmp(QTest::currentDataTag(), "get_large_16bits") == 0
        || qstrcmp(QTest::currentDataTag(), "get_large_16bits_confirmable") == 0) {
        client.setBlockSize(16);
    }

    request.setType(type);
    QCoapReply *reply = client.get(request);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    QSignalSpy spyReplyError(reply, SIGNAL(error(QCoapReply::NetworkError)));
    Helper helper;
    connect(reply, SIGNAL(error(QCoapReply::NetworkError)), &helper, SLOT(onError(QCoapReply::NetworkError)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);
    QCOMPARE(spyReplyError.count(), 0);

    QByteArray dataReply = reply->readAll();
    QCOMPARE(dataReply, replyData);
}

void tst_QCoapClient::blockwiseRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<QByteArray>("requestData");
    QTest::addColumn<QCoapReply::StatusCode>("statusCode");
    QTest::addColumn<QByteArray>("replyData");

    QByteArray data;
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");

    QTest::newRow("large_post_empty_reply") << QUrl("coap://172.17.0.3:5683/query")
                               << QCoapMessage::NonConfirmable
                               << data
                               << QCoapReply::MethodNotAllowed
                               << QByteArray();
    QTest::newRow("large_post_large_reply") << QUrl("coap://172.17.0.3:5683/large-post")
                               << QCoapMessage::NonConfirmable
                               << data
                               << QCoapReply::Changed
                               << data.toUpper();
}

void tst_QCoapClient::blockwiseRequest()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(QByteArray, requestData);
    QFETCH(QCoapReply::StatusCode, statusCode);
    QFETCH(QByteArray, replyData);

    QCoapClient client;
    client.setBlockSize(16);

    QCoapRequest request(url);
    request.setType(type);
    request.addOption(QCoapOption::ContentFormat);

    QScopedPointer<QCoapReply> reply(client.post(request, requestData));
    QSignalSpy spyReplyFinished(reply.data(), SIGNAL(finished()));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);

    QByteArray dataReply = reply->readAll();
    QCOMPARE(dataReply, replyData);
    QCOMPARE(reply->statusCode(), statusCode);
}

void tst_QCoapClient::discover_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<int>("resourceNumber");

    QTest::newRow("discover") << QUrl("coap://172.17.0.3:5683/")
                              << 29;
}

void tst_QCoapClient::discover()
{
    QFETCH(QUrl, url);
    QFETCH(int, resourceNumber);

    QCoapClient client;

    QScopedPointer<QCoapDiscoveryReply> resourcesReply(client.discover(url)); // /.well-known/core
    QSignalSpy spyReplyFinished(resourcesReply.data(), SIGNAL(finished()));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);
    QCOMPARE(resourcesReply->resourceList().length(), resourceNumber);
}

void tst_QCoapClient::observe_data()
{
    QWARN("Observe tests may take some times...");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::MessageType>("type");

    QTest::newRow("observe") << QUrl("coap://172.17.0.3:5683/obs")
                             << QCoapMessage::NonConfirmable;
    QTest::newRow("observe_confirmable") << QUrl("coap://172.17.0.3:5683/obs")
                                         << QCoapMessage::Confirmable;
    QTest::newRow("observe_receive_non") << QUrl("coap://172.17.0.3:5683/obs-non")
                                         << QCoapMessage::NonConfirmable;
    QTest::newRow("observe_receive_non_confirmable") << QUrl("coap://172.17.0.3:5683/obs-non")
                                                     << QCoapMessage::Confirmable;
    QTest::newRow("observe_large") << QUrl("coap://172.17.0.3:5683/obs-large")
                                   << QCoapMessage::NonConfirmable;
    QTest::newRow("observe_large_confirmable") << QUrl("coap://172.17.0.3:5683/obs-large")
                                               << QCoapMessage::Confirmable;
    QTest::newRow("observe_pumping") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                     << QCoapMessage::NonConfirmable;
    QTest::newRow("observe_pumping_confirmable") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                                 << QCoapMessage::Confirmable;
}

void tst_QCoapClient::observe()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::MessageType, type);

    QCoapClient client;
    QCoapRequest request(url);

    request.setType(type);
    QScopedPointer<QCoapReply> reply(client.observe(request));
    QSignalSpy spyReplyNotified(reply.data(), SIGNAL(notified(const QByteArray&)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.count(), 3, 30000);
    for (QList<QVariant> receivedSignals : qAsConst(spyReplyNotified)) {
#if 0
        qDebug() << receivedSignals.first().toByteArray();
#else
        Q_UNUSED(receivedSignals)
#endif
    }

    client.cancelObserve(reply.data());
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.count(), 4, 42000);
    for (QList<QVariant> receivedSignals : qAsConst(spyReplyNotified)) {
#if 0
        qDebug() << receivedSignals.first().toByteArray();
#else
        Q_UNUSED(receivedSignals)
#endif
    }
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
