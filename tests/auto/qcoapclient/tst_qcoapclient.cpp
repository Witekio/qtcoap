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

    void sendNewRequest(QCoapRequest* req) { sendRequest(req); }

private:
    QList<QCoapRequest> requests;
};

void tst_QCoapClient::uniqueTokensAndMessageIds()
{
    QCoapClientForTests client;

    QList<QByteArray> tokenList;
    QList<quint16> ids;
    for (int i = 0; i < 500; ++i) {
        QCoapRequest* request = new QCoapRequest();
        client.sendNewRequest(request);
        QVERIFY(!tokenList.contains(request->token()));
        QVERIFY(!ids.contains(request->messageId()));
        tokenList.push_back(request->token());
        ids.push_back(request->messageId());
    }
}

void tst_QCoapClient::get_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
}

void tst_QCoapClient::get()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QSignalSpy spyClientFinished(&client, SIGNAL(finished()));

    QCoapRequest* request = new QCoapRequest(url);
    QCoapReply* reply = client.get(request);
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 1000);

    QVERIFY(reply != nullptr);
    QCOMPARE(request->token(), reply->token());

    delete request;
    delete reply;
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
