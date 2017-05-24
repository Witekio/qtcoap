#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapconnection.h"
#include "qcoaprequest.h"

class tst_QCoapConnection : public QObject
{
    Q_OBJECT

public:
    tst_QCoapConnection();
    ~tst_QCoapConnection();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void ctor_data();
    void ctor();
    void sendRequest_data();
    void sendRequest();
    void readReply_data();
    void readReply();
};

tst_QCoapConnection::tst_QCoapConnection()
{
}

tst_QCoapConnection::~tst_QCoapConnection()
{
}

void tst_QCoapConnection::initTestCase()
{
}

void tst_QCoapConnection::cleanupTestCase()
{
}

void tst_QCoapConnection::ctor_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<int>("port");

    QTest::newRow("hostAndPort") << "test-server" << 5684;
    QTest::newRow("hostOnly") << "test-server" << 5683;
    QTest::newRow("nothing") << "localhost" << 5683;
}

void tst_QCoapConnection::ctor()
{
    /*QFETCH(QString, host);
    QFETCH(int, port);

    QCoapConnection* connection;

    if (qstrcmp(QTest::currentDataTag(), "nothing") == 0)
        connection = new QCoapConnection();
    else if (qstrcmp(QTest::currentDataTag(), "hostOnly") == 0)
        connection = new QCoapConnection(host);
    else
        connection = new QCoapConnection(host, port);

    QCOMPARE(connection->host(), host);
    QCOMPARE(connection->port(), port);*/
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<ushort>("port");
    QTest::addColumn<QString>("statusCode");

    QTest::newRow("success") << "coap://" << "test-server" << "/temperature" << ushort(5683);
    QTest::newRow("failure-path") << "coap://" << "test-server" << "/path-which-fail" << ushort(5683);
    QTest::newRow("failure-protocol") << "http://" << "test-server" << "/temperature" << ushort(5683);
}

void tst_QCoapConnection::sendRequest()
{
    /*QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(ushort, port);
    QFETCH(int, statusCode);

    QCoapConnection connection(host, port);

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(connection, SIGNAL(readyRead()));

    QCoapRequest request(protocol + host + path);
    connection.sendRequest(request);

    // TODO HERE :
    // read if something is write to the socket
    // and check status code

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 35000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 35000);*/
}

void tst_QCoapConnection::readReply_data()
{
    /*QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<ushort>("port");

    QTest::newRow("success") << "coap://" << "test-server" << "/temperature" << ushort(5683);*/
}

void tst_QCoapConnection::readReply()
{
    /*QCoapConnection connection(host, port);

    QCoapRequest request(protocol + host + path);
    client.sendRequest(request);

    QVERIFY(!connection.readReply().isEmpty());*/
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
