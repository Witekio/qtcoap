#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapclient.h"
#include "qcoapclient_p.h"
#include "qcoapconnection_p.h"
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
        QCoapClientPrivate* privateClient = static_cast<QCoapClientPrivate*>(d_func());
        privateClient->setConnection(new QCoapConnectionForSocketErrorTests);
        privateClient->connection->moveToThread(privateClient->workerThread);
    }

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
    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply*)));

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
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);
    QByteArray replyData;
    if (reply) {
        replyData = reply->readAll();
        if (qstrcmp(QTest::currentDataTag(), "get") == 0) {
            QVERIFY(!replyData.isEmpty());
            QCOMPARE(reply->statusCode(), ContentCoapCode);
        } else if (qstrcmp(QTest::currentDataTag(), "post") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), CreatedCoapCode);
        } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), ChangedCoapCode);
        } else if (qstrcmp(QTest::currentDataTag(), "delete") == 0) {
            QVERIFY(replyData.isEmpty());
            QCOMPARE(reply->statusCode(), DeletedCoapCode);
        }

        delete reply;
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

    //QFAIL("Comment the QFAIL");

    QCoapClient client;
    QCoapRequest request(url);

    QCoapReply* reply = client.get(request);

    QSignalSpy spyReplyFinished(reply, SIGNAL(finished()));
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);
    QByteArray replyData = reply->readAll();

    QVERIFY(!replyData.isEmpty());
    QCOMPARE(reply->statusCode(), ContentCoapCode);

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
    buffer.open(QIODevice::ReadWrite);
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
        QCOMPARE(reply->statusCode(), CreatedCoapCode);
    } else if (qstrcmp(QTest::currentDataTag(), "put") == 0) {
        QVERIFY(replyData.isEmpty());
        QCOMPARE(reply->statusCode(), ChangedCoapCode);
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
    QSignalSpy spyClientFinished(&client, SIGNAL(finished(QCoapReply*)));
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
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 4, 5000);

    QByteArray replyData1 = replyGet1->readAll();
    QByteArray replyData2 = replyGet2->readAll();
    QByteArray replyData3 = replyGet3->readAll();
    QByteArray replyData4 = replyGet4->readAll();

    QVERIFY(replyData1 != replyData2);
    QVERIFY(replyData1 != replyData3);
    QVERIFY(replyData1 != replyData4);
    QVERIFY(replyData2 != replyData3);
    QVERIFY(replyData2 != replyData4);
    QVERIFY(replyData3 != replyData4);
    QCOMPARE(replyGet1->statusCode(), ContentCoapCode);
    QCOMPARE(replyGet2->statusCode(), ContentCoapCode);
    QCOMPARE(replyGet3->statusCode(), ContentCoapCode);
    QCOMPARE(replyGet4->statusCode(), ContentCoapCode);
}

void tst_QCoapClient::socketError()
{
    QCoapClientForSocketErrorTests client;
    QUrl url = QUrl("coap://172.17.0.3:5683/test");

    QUdpSocket* socket = client.connection()->socket();
    QSignalSpy spySocketError(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QCoapReply* reply = client.get(QCoapRequest(url));
    QSignalSpy spyReplyError(reply, SIGNAL(error(QCoapReply::QCoapNetworkError)));

    QTRY_COMPARE_WITH_TIMEOUT(spySocketError.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyReplyError.count(), 1, 10000);
    QCOMPARE(spyReplyError.first().first(), QCoapReply::AddressInUseCoapError);
}

void tst_QCoapClient::timeout()
{
    QWARN("Timeout test may take some times...");

    QCoapClient client;
    client.protocol()->setAckTimeout(300);
    QUrl url = QUrl("coap://172.17.0.5:5683/"); // Need an url that return nothing

    QCoapReply* reply = client.get(QCoapRequest(url, QCoapMessage::ConfirmableCoapMessage));
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
                               << QCoapMessage::NonConfirmableCoapMessage
                               << data;
    QTest::newRow("get_large_separate") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::NonConfirmableCoapMessage
                               << data;
    QTest::newRow("get_large_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::ConfirmableCoapMessage
                               << data;
    QTest::newRow("get_large_separate_confirmable") << QUrl("coap://172.17.0.3:5683/large-separate")
                               << QCoapMessage::ConfirmableCoapMessage
                               << data;
    QTest::newRow("get_large_16bits") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::NonConfirmableCoapMessage
                               << data;
    QTest::newRow("get_large_16bits_confirmable") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapMessage::ConfirmableCoapMessage
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
                               << QCoapMessage::NonConfirmableCoapMessage
                               << data
                               << MethodNotAllowedCoapCode
                               << QByteArray();
    QTest::newRow("large_post_large_reply") << QUrl("coap://172.17.0.3:5683/large-post")
                               << QCoapMessage::NonConfirmableCoapMessage
                               << data
                               << ChangedCoapCode
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
    request.addOption(QCoapOption::ContentFormatCoapOption);

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
                             << QCoapMessage::NonConfirmableCoapMessage;
    QTest::newRow("observe_confirmable") << QUrl("coap://172.17.0.3:5683/obs")
                                         << QCoapMessage::ConfirmableCoapMessage;
    QTest::newRow("observe_receive_non") << QUrl("coap://172.17.0.3:5683/obs-non")
                                         << QCoapMessage::NonConfirmableCoapMessage;
    QTest::newRow("observe_receive_non_confirmable") << QUrl("coap://172.17.0.3:5683/obs-non")
                                                     << QCoapMessage::ConfirmableCoapMessage;
    QTest::newRow("observe_large") << QUrl("coap://172.17.0.3:5683/obs-large")
                                   << QCoapMessage::NonConfirmableCoapMessage;
    QTest::newRow("observe_large_confirmable") << QUrl("coap://172.17.0.3:5683/obs-large")
                                               << QCoapMessage::ConfirmableCoapMessage;
    QTest::newRow("observe_pumping") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                     << QCoapMessage::NonConfirmableCoapMessage;
    QTest::newRow("observe_pumping_confirmable") << QUrl("coap://172.17.0.3:5683/obs-pumping")
                                                 << QCoapMessage::ConfirmableCoapMessage;
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
