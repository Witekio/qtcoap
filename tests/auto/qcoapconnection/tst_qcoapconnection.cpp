#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <QtNetwork/QUdpSocket>
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
    void connectToHost_data();
    void connectToHost();
    void sendRequest_data();
    void sendRequest();
    void writeToSocket_data();
    void writeToSocket();
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
    QFETCH(QString, host);
    QFETCH(int, port);

    QCoapConnection* connection;

    if (qstrcmp(QTest::currentDataTag(), "nothing") == 0)
        connection = new QCoapConnection();
    else if (qstrcmp(QTest::currentDataTag(), "hostOnly") == 0)
        connection = new QCoapConnection(host);
    else
        connection = new QCoapConnection(host, port);

    QCOMPARE(connection->host(), host);
    QCOMPARE(connection->port(), port);
    QVERIFY(connection->socket() != nullptr);

    delete connection;
}

void tst_QCoapConnection::connectToHost_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("port");
    //QTest::addColumn<QString>("statusCode");

    QTest::newRow("success") << "coap://" << "test-server" << "/temperature" << 5683;
    QTest::newRow("failure") << "coap://" << "not-a-test-server" << "/will-fail" << 5683;
}

void tst_QCoapConnection::connectToHost()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);

    QCoapConnection connection(host, port);

    QSignalSpy spySocketHostFound(connection.socket(), SIGNAL(hostFound()));
    QSignalSpy spySocketConnected(connection.socket(), SIGNAL(connected()));
    QSignalSpy SpySocketError(connection.socket(), SIGNAL(error()));

    QCOMPARE(connection.state(), QCoapConnection::UNCONNECTED);

    connection.connectToHost();

    if (qstrcmp(QTest::currentDataTag(), "success") == 0) {
        QTRY_COMPARE_WITH_TIMEOUT(spySocketHostFound.count(), 1, 5000);
        QTRY_COMPARE_WITH_TIMEOUT(spySocketConnected.count(), 1, 5000);
        QCOMPARE(connection.state(), QCoapConnection::CONNECTED);
    }
    else {
        QTRY_COMPARE_WITH_TIMEOUT(SpySocketError.count(), 1, 5000);
        QCOMPARE(connection.state(), QCoapConnection::UNCONNECTED);
    }
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("port");
    QTest::addColumn<QString>("data");

    // TODO : change data to match the result we expect
    QTest::newRow("simple_request") << "coap://" << "test-server" << "/temperature" << 5683 << "data";
}

void tst_QCoapConnection::sendRequest()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);
    QFETCH(QString, data);

    QCoapConnection connection(host, port);

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(&connection, SIGNAL(readyRead()));

    QCoapRequest request(protocol + host + path);
    connection.sendRequest(request.toPdu());
    QVERIFY(connection.socket() != nullptr);
    QVERIFY(!connection.socket()->readAll().isEmpty());

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 5000);

    QCOMPARE(connection.readReply(), data.toUtf8());
}

class QCoapConnectionForTest : public QCoapConnection
{
public:
    QCoapConnectionForTest(const QString& host = "localhost",
                           int port = 5683,
                           QObject* parent = nullptr) :
        QCoapConnection(host, port, parent)
    {}

    void setSocket(QIODevice* device)
    {
        udpSocket = device;
    }
};

void tst_QCoapConnection::writeToSocket_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("success") << "Data for the writing/reading to a socket(QIODevice) test";
}

void tst_QCoapConnection::writeToSocket()
{
    QFETCH(QString, data);

    QCoapConnectionForTest connection;
    QBuffer socket;

    socket.write(data.toUtf8());
    connection.setSocket(&socket);

    QCOMPARE(connection.readReply(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
