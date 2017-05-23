#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

class QCoapClient : public QObject
{
    Q_OBJECT

public:
    QCoapClient();
    ~QCoapClient();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

QCoapClient::QCoapClient()
{

}

QCoapClient::~QCoapClient()
{

}

void QCoapClient::initTestCase()
{

}

void QCoapClient::cleanupTestCase()
{

}

void QCoapClient::test_case1()
{

}

QTEST_MAIN(QCoapClient)

#include "tst_qcoapclient.moc"
