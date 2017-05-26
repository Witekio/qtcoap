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
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("port");
    //QTest::addColumn<QString>("statusCode");

    QTest::newRow("success") << "coap://" << "test-server" << "/temperature" << 5683;
    QTest::newRow("failure-path") << "coap://" << "test-server" << "/path-which-fail" << 5683;
    QTest::newRow("failure-protocol") << "http://" << "test-server" << "/temperature" << 5683;
}

void tst_QCoapConnection::sendRequest()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);
    //QFETCH(QString, statusCode);

    QCoapConnection connection(host, port);

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(&connection, SIGNAL(readyRead()));

    QCoapRequest request(protocol + host + path);
    connection.sendRequest(request);

    // TODO HERE :
    // read if something is written to the socket
    // and check status code
    QVERIFY(connection.socket() != nullptr);
    QVERIFY(!connection.socket()->readAll().isEmpty());

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 5000);
}

class QCoapConnectionForTest : public QCoapConnection
{
public:
    QCoapConnectionForTest(const QString& host = "localhost",
                           int port = 5683,
                           QObject* parent = nullptr) :
        QCoapConnection(host, port, parent)
    {}

    void setSocket(QUdpSocket* socket)
    {
        udpSocket = socket;
    }
};

void tst_QCoapConnection::readReply_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("port");
    QTest::addColumn<QString>("data");

    QTest::newRow("success") << "coap://" << "test-server" << "/temperature" << 5683 << "Data for the reading test";
}

void tst_QCoapConnection::readReply()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);
    QFETCH(QString, data);

    QCoapConnectionForTest connection(host, port);
    QUdpSocket socket;

    socket.write(data.toUtf8());
    connection.setSocket(&socket);

    QCOMPARE(connection.readReply(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
