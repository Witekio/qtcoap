#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapclient.h"
#include "qcoaprequest.h"
#include "qcoapreply.h"
#include <QBuffer>

Q_DECLARE_METATYPE(QCoapOperation)
Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType)

class tst_QCoapClient : public QObject
{
    Q_OBJECT

public:
    tst_QCoapClient();
    ~tst_QCoapClient();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void operations_data();
    void operations();
    void separateOperation_data();
    void separateOperation();
    void removeReply_data();
    void removeReply();
    void requestWithQIODevice_data();
    void requestWithQIODevice();
    void multipleRequests_data();
    void multipleRequests();
    void blockwiseReply_data();
    void blockwiseReply();
    void discover_data();
    void discover();
    void observe_data();
    void observe();
};

tst_QCoapClient::tst_QCoapClient()
{
}

tst_QCoapClient::~tst_QCoapClient()
{
}

void tst_QCoapClient::initTestCase()
{
}

void tst_QCoapClient::cleanupTestCase()
{
}

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

    //QFAIL("Uncomment the QFAIL");

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply* reply = nullptr;
    if (qstrcmp(QTest::currentDataTag(), "get") == 0)
        reply = client.get(request);
    else if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply = client.post(request);
    else if (qstrcmp(QTest::currentDataTag(), "put") == 0)
        reply = client.put(request);
    else if (qstrcmp(QTest::currentDataTag(), "delete") == 0)
        reply = client.deleteResource(request);

    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);
    QByteArray replyData = reply->readAll();

    if (qstrcmp(QTest::currentDataTag(), "get") == 0) {
        QVERIFY(!replyData.isEmpty());
        QCOMPARE(reply->statusCode(), ContentCode);
    } else if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), CreatedCode);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), ChangedCode);
    } else if (qstrcmp(QTest::currentDataTag(), "delete") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), DeletedCode);
    }

    delete reply;
}

void tst_QCoapClient::separateOperation_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/separate");
}

void tst_QCoapClient::separateOperation()
{
    QFETCH(QUrl, url);

    //QFAIL("Uncomment the QFAIL");

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply* reply = client.get(request);

    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);
    QByteArray replyData = reply->readAll();

    QVERIFY(!replyData.isEmpty());
    QCOMPARE(reply->statusCode(), ContentCode);

    delete reply;
}

void tst_QCoapClient::removeReply_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/test");
}

void tst_QCoapClient::removeReply()
{
    QFETCH(QUrl, url);

    QFAIL("Comment the QFAIL to test it");

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply* reply = nullptr;
    reply = client.get(request);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    delete reply;
    reply = 0;

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

    QCoapReply* reply = nullptr;
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    buffer.write("Some data");

    if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply = client.post(request, &buffer);
    else
        reply = client.put(request, &buffer);

    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);
    QByteArray replyData = reply->readAll();

    if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), CreatedCode);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), ChangedCode);
    }

    delete reply;
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
    //QCoapRequest request(url);

    QCoapReply* replyGet1 = client.get(QCoapRequest(url));
    QSignalSpy spyReplyGet1Finished(replyGet1, SIGNAL(finished()));
    QCoapReply* replyGet2 = client.get(QCoapRequest(url));
    QSignalSpy spyReplyGet2Finished(replyGet2, SIGNAL(finished()));
    QCoapReply* replyGet3 = client.get(QCoapRequest(url));
    QSignalSpy spyReplyGet3Finished(replyGet3, SIGNAL(finished()));
    QCoapReply* replyGet4 = client.get(QCoapRequest(url));
    QSignalSpy spyReplyGet4Finished(replyGet4, SIGNAL(finished()));

    QVERIFY(replyGet1 != nullptr);
    QVERIFY(replyGet2 != nullptr);
    QVERIFY(replyGet3 != nullptr);
    QVERIFY(replyGet4 != nullptr);

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet1Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet2Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet3Finished.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyGet4Finished.count(), 1, 5000);

    QByteArray replyGet1Data = replyGet1->readAll();
    QByteArray replyGet2Data = replyGet2->readAll();
    QByteArray replyGet3Data = replyGet3->readAll();
    QByteArray replyGet4Data = replyGet4->readAll();
    qDebug() << "replyGet1Data : " << replyGet1Data;
    qDebug() << "replyGet2Data : " << replyGet2Data;
    qDebug() << "replyGet3Data : " << replyGet3Data;
    qDebug() << "replyGet4Data : " << replyGet4Data;

    QVERIFY(!replyGet1Data.isEmpty());
    QCOMPARE(replyGet1->statusCode(), ContentCode);
    QVERIFY(!replyGet2Data.isEmpty());
    QCOMPARE(replyGet2->statusCode(), ContentCode);
    QVERIFY(!replyGet3Data.isEmpty());
    QCOMPARE(replyGet3->statusCode(), ContentCode);
    QVERIFY(!replyGet4Data.isEmpty());
    QCOMPARE(replyGet4->statusCode(), ContentCode);
}

void tst_QCoapClient::blockwiseReply_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
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
                               << QCoapMessage::NONCONFIRMABLE
                               << data;
    QTest::newRow("get_large_separate") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::NONCONFIRMABLE
                               << data;
    QTest::newRow("get_large_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::CONFIRMABLE
                               << data;
    QTest::newRow("get_large_separate_confirmable") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::CONFIRMABLE
                               << data;
    QTest::newRow("get_large_16bits") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::NONCONFIRMABLE
                               << data;
    QTest::newRow("get_large_16bits_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::CONFIRMABLE
                               << data;
}

void tst_QCoapClient::blockwiseReply()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(QByteArray, replyData);

    QCoapClient client;
    QCoapRequest request(url);

    if (qstrcmp(QTest::currentDataTag(), "get_large_16bits") == 0
        || qstrcmp(QTest::currentDataTag(), "get_large_16bits_confirmable") == 0)
        client.setBlockSize(16);

    request.setType(type);
    QCoapReply* reply = client.get(request);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);

    QByteArray dataReply = reply->readAll();
    QCOMPARE(dataReply, replyData);
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

    //QFAIL("Broken when protocol added");
    for (int i = 0; i < 20; ++i) {
        QCoapClient client;

        QCoapDiscoveryReply* resourcesReply = client.discover(url); // /.well-known/core
        QSignalSpy spyReplyFinished(resourcesReply, SIGNAL(finished()));

        QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);
        QCOMPARE(resourcesReply->resourceList().length(), resourceNumber);
    }
}

void tst_QCoapClient::observe_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");

    QTest::newRow("observe") << QUrl("coap://172.17.0.3:5683/obs")
                             << QCoapMessage::NONCONFIRMABLE;
    QTest::newRow("observe_confirmable") << QUrl("coap://172.17.0.3:5683/obs")
                                         << QCoapMessage::CONFIRMABLE;
    QTest::newRow("observe_receive_non") << QUrl("coap://172.17.0.3:5683/obs-non")
                             << QCoapMessage::NONCONFIRMABLE;
    QTest::newRow("observe_receive_non_confirmable") << QUrl("coap://172.17.0.3:5683/obs-non")
                                         << QCoapMessage::CONFIRMABLE;
    QTest::newRow("observe_large") << QUrl("coap://172.17.0.3:5683/obs-large")
                                         << QCoapMessage::NONCONFIRMABLE;

}

void tst_QCoapClient::observe()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::QCoapMessageType, type);

    //QFAIL("Does not work for instance");
    QCoapClient client;
    QCoapRequest request(url);
    QCoapReply* reply = nullptr;

    request.setType(type);
    reply = client.observe(request);
    QSignalSpy spyReplyNotified(reply, SIGNAL(notified(const QByteArray&)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.count(), 3, 30000);
    for (QList<QVariant> receivedSignals : spyReplyNotified) {
        qDebug() << receivedSignals.first().toByteArray();
    }

    client.cancelObserve(reply);
    QThread::sleep(12);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyNotified.count(), 4, 30000);
    for (QList<QVariant> receivedSignals : spyReplyNotified) {
        qDebug() << receivedSignals.first().toByteArray();
    }

    if (reply)
        delete reply;
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
