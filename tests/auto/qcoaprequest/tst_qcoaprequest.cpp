#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <qcoaprequest.h>

class tst_QCoapRequest : public QObject
{
    Q_OBJECT

public:
    tst_QCoapRequest();
    ~tst_QCoapRequest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void setUrl_data();
    void setUrl();
};

tst_QCoapRequest::tst_QCoapRequest()
{

}

tst_QCoapRequest::~tst_QCoapRequest()
{

}

void tst_QCoapRequest::initTestCase()
{

}

void tst_QCoapRequest::cleanupTestCase()
{

}

void tst_QCoapRequest::setUrl_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("empty") << QUrl();
    QTest::newRow("coap") << QUrl("coap://test-server/temperature");
}

void tst_QCoapRequest::setUrl()
{
    QFETCH(QUrl, url);

    QCoapRequest request;
    request.setUrl(url);
    QCOMPARE(request.url(), url);
}

QTEST_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
