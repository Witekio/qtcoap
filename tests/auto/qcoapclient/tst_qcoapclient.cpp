#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapclient.h"
#include "qcoaprequest.h"
#include "qcoapreply.h"
#include <QBuffer>

Q_DECLARE_METATYPE(QCoapStatusCode)
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
    // TODO : void retransmission(); HOW ?
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

    //QFAIL("Comment the QFAIL");

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

    //QFAIL("Comment the QFAIL to test it");

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply* reply = nullptr;
    reply = client.get(request);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    delete reply;
    reply = 0;

    QThread::sleep(5);
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

void tst_QCoapClient::timeout()
{
    QWARN("Timeout test may take some times...");

    QCoapClient client;
    client.protocol()->setAckTimeout(300);
    QUrl url = QUrl("coap://172.17.0.5:5683/"); // Need an url that return nothing

    QCoapReply* reply = client.get(QCoapRequest(url));
    QSignalSpy spyReplyError(reply, SIGNAL(error(QCoapReply::QCoapNetworkError)));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.count(), 1, 60000);
    QCOMPARE(spyReplyError.first().first(), QCoapReply::TimeOutCoapError);
}

void tst_QCoapClient::abort()
{
    QCoapClient client;
    QUrl url = QUrl("coap://172.17.0.3:5683/large");

    QCoapReply* replyGet1 = client.get(QCoapRequest(url));
    QSignalSpy spyReplyGet1Finished(replyGet1, SIGNAL(finished()));
    replyGet1->abortRequest();

    QThread::sleep(1);
    QCOMPARE(spyReplyGet1Finished.count(), 0);
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
                               << QCoapMessage::NonConfirmableMessage
                               << data;
    QTest::newRow("get_large_separate") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::NonConfirmableMessage
                               << data;
    QTest::newRow("get_large_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::ConfirmableMessage
                               << data;
    QTest::newRow("get_large_separate_confirmable") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::ConfirmableMessage
                               << data;
    QTest::newRow("get_large_16bits") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::NonConfirmableMessage
                               << data;
    QTest::newRow("get_large_16bits_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::ConfirmableMessage
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

void tst_QCoapClient::blockwiseRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<QByteArray>("requestData");
    QTest::addColumn<QCoapStatusCode>("statusCode");
    QTest::addColumn<QByteArray>("replyData");

    QByteArray data;
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
    data.append("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");

    QTest::newRow("large_post_empty_reply") << QUrl("coap://172.17.0.3:5683/query")
                               << QCoapMessage::NonConfirmableMessage
                               << data
                               << MethodNotAllowedCode
                               << QByteArray();
    QTest::newRow("large_post_large_reply") << QUrl("coap://172.17.0.3:5683/large-post")
                               << QCoapMessage::NonConfirmableMessage
                               << data
                               << ChangedCode
                               << data.toUpper();
}

void tst_QCoapClient::blockwiseRequest()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(QByteArray, requestData);
    QFETCH(QCoapStatusCode, statusCode);
    QFETCH(QByteArray, replyData);

    QCoapClient client;
    client.setBlockSize(16);

    QCoapRequest request(url);
    request.setType(type);
    request.addOption(QCoapOption::ContentFormatOption);

    QCoapReply* reply = client.post(request, requestData);
    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));

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

    //QFAIL("Broken when protocol added");
    QCoapClient client;

    QCoapDiscoveryReply* resourcesReply = client.discover(url); // /.well-known/core
    QSignalSpy spyReplyFinished(resourcesReply, SIGNAL(finished()));

    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 30000);
    QCOMPARE(resourcesReply->resourceList().length(), resourceNumber);
}

void tst_QCoapClient::observe_data()
{
    QWARN("Observe tests may take some times...");
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");

    QTest::newRow("observe") << QUrl("coap://172.17.0.3:5683/obs")
                             << QCoapMessage::NonConfirmableMessage;
    QTest::newRow("observe_confirmable") << QUrl("coap://172.17.0.3:5683/obs")
                                         << QCoapMessage::ConfirmableMessage;
    QTest::newRow("observe_receive_non") << QUrl("coap://172.17.0.3:5683/obs-non")
                                         << QCoapMessage::NonConfirmableMessage;
    QTest::newRow("observe_receive_non_confirmable") << QUrl("coap://172.17.0.3:5683/obs-non")
                                                     << QCoapMessage::ConfirmableMessage;
    QTest::newRow("observe_large") << QUrl("coap://172.17.0.3:5683/obs-large")
                                   << QCoapMessage::NonConfirmableMessage;
    QTest::newRow("observe_large_confirmable") << QUrl("coap://172.17.0.3:5683/obs-large")
                                               << QCoapMessage::ConfirmableMessage;
    QTest::newRow("observe_pumping") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                     << QCoapMessage::NonConfirmableMessage;
    QTest::newRow("observe_pumping_confirmable") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                                 << QCoapMessage::ConfirmableMessage;
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
