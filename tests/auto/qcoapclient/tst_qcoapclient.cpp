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
    void get_data();
    void get();
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

void tst_QCoapClient::get_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("post") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("put") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("delete") << QUrl("coap://172.17.0.3:5683/test");
    QTest::newRow("discover") << QUrl("coap://172.17.0.3:5683/.well-known/core");
}

void tst_QCoapClient::get()
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
    else reply = client.get(request);

    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyRequestFinished.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 10000);

    QVERIFY(reply != nullptr);
    QCOMPARE(request->token(), reply->token());

    delete request;
    delete reply;
}

//void tst_QCoapClient::put();

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
