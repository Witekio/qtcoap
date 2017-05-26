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

void tst_QCoapClient::get_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("get") << QUrl("coap://test-server:5683/temperature");
}

void tst_QCoapClient::get()
{
    QFETCH(QUrl, url);

    QCoapClient client;
    QSignalSpy spyClientFinished(&client, SIGNAL(finished()));

    //QCoapRequest* request = new QCoapRequest(url);
    QCoapReply* reply = client.get(QCoapRequest(url));
    QTRY_COMPARE_WITH_TIMEOUT(spyClientFinished.count(), 1, 5000);

    QVERIFY(reply != nullptr);

    delete reply;
    //delete request;
}

QTEST_MAIN(tst_QCoapClient)

#include "tst_qcoapclient.moc"
