#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapclient.h"
#include "qcoaprequest.h"
#include "qcoapreply.h"

class tst_QCoapClient : public QObject
{
    Q_OBJECT

public:
    tst_QCoapClient();
    ~tst_QCoapClient();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void uniqueTokensAndMessageIds();
    void operations_data();
    void operations();
    void discover_data();
    void discover();
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

class QCoapClientForTests : public QCoapClient
{
public:
    QCoapClientForTests() : QCoapClient() {}

    void addNewRequest(QCoapRequest* req) { addRequest(req); }
};

void tst_QCoapClient::uniqueTokensAndMessageIds()
{
    QCoapClientForTests client;

    QList<QByteArray> tokenList;
    QList<quint16> ids;
    for (int i = 0; i < 100; ++i) {
        QCoapRequest* request = new QCoapRequest();
        client.addNewRequest(request);
        QVERIFY(!tokenList.contains(request->token()));
        QVERIFY(!ids.contains(request->messageId()));
        tokenList.push_back(request->token());
        ids.push_back(request->messageId());
    }
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

    QCoapClientForTests client;
    QCoapRequest* request = new QCoapRequest(url);

    QSignalSpy spyClientFinished(&client, SIGNAL(finished()));
    QSignalSpy spyRequestFinished(request, SIGNAL(finished(QCoapRequest*)));
    QSignalSpy spyConnectionReadyRead(request->connection(), SIGNAL(readyRead()));

    QCoapReply* reply;
    if (qstrcmp(QTest::currentDataTag(), "get") == 0)
        reply = client.get(request);
    else if (qstrcmp(QTest::currentDataTag(), "post") == 0)
        reply = client.post(request);
    else if (qstrcmp(QTest::currentDataTag(), "put") == 0)
        reply = client.put(request);
    else if (qstrcmp(QTest::currentDataTag(), "delete") == 0)
        reply = client.deleteResource(request);

    QTRY_VERIFY_WITH_TIMEOUT(spyConnectionReadyRead.count() > 0, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyRequestFinished.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 10000);

    QVERIFY(reply != nullptr);
    QCOMPARE(request->token(), reply->token());

    delete request;
    delete reply;
}

void tst_QCoapClient::discover_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QByteArray>("dataReply");

    QTest::newRow("discover") << QUrl("coap://172.17.0.3:5683/")
                              << QByteArray("</obs>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</obs-pumping>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</separate>;title=\"Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way\",</large-create>;rt=\"block\";title=\"Large resource that can be created using POST method\",</seg1>;title=\"Long path resource\",</seg1/seg2>;title=\"Long path resource\",</seg1/seg2/seg3>;title=\"Long path resource\",</large-separate>;rt=\"block\";sz=1280;title=\"Large resource\",</obs-reset>,</.well-known/core>,</multi-format>;ct=\"0 41\";title=\"Resource that exists in different content formats (text/plain utf8 and application/xml)\",</path>;ct=40;title=\"Hierarchical link description entry\",</path/sub1>;title=\"Hierarchical link description sub-resource\",</path/sub2>;title=\"Hierarchical link description sub-resource\",</path/sub3>;title=\"Hierarchical link description sub-resource\",</link1>;if=\"If1\";rt=\"Type1 Type2\";title=\"Link test resource\",</link3>;if=\"foo\";rt=\"Type1 Type3\";title=\"Link test resource\",</link2>;if=\"If2\";rt=\"Type2 Type3\";title=\"Link test resource\",</obs-large>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</validate>;title=\"Resource which varies\",</test>;title=\"Default test resource\",</large>;rt=\"block\";sz=1280;title=\"Large resource\",</obs-pumping-non>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</query>;title=\"Resource accepting query parameters\",</large-post>;rt=\"block\";title=\"Handle POST with two-way blockwise transfer\",</location-query>;title=\"Perform POST transaction with responses containing several Location-Query options (CON mode)\",</obs-non>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</large-update>;rt=\"block\";sz=1280;title=\"Large resource that can be updated using PUT method\",</shutdown>");
}

void tst_QCoapClient::discover()
{
    QFETCH(QUrl, url);
    QFETCH(QByteArray, dataReply);

    QCoapClientForTests client;

    QCoapReply* reply = client.discover(url); // /.well-known/core

    QTRY_COMPARE_WITH_TIMEOUT(reply->readData(), dataReply, 10000);

    delete reply;
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
