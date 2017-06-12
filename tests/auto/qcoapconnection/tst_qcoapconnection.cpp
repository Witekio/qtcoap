#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <QUdpSocket>
#include <QBuffer>
#include <QtCore/qglobal.h>
#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

Q_DECLARE_METATYPE(QCoapRequest::QCoapRequestOperation)

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

class QCoapConnectionForTest : public QCoapConnection
{
    Q_OBJECT
public:
    QCoapConnectionForTest(const QString& host = "localhost",
                           int port = 5683,
                           QObject* parent = nullptr) :
        QCoapConnection(host, port, parent)
    {}

    void connectToHostForTest() { connectToHost(); }
    void setSocketForTest(QIODevice* device) { setSocket(device); }
    void writeToSocketForTests(const QByteArray& data) { writeToSocket(data); }
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

    QTest::newRow("success") << "coap://" << "vs0.inf.ethz.ch" << "/test" << 5683;
    QTest::newRow("failure") << "coap://" << "not-a-test-server" << "/will-fail" << 5683;
}

void tst_QCoapConnection::connectToHost()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);

    QCoapConnectionForTest connection(host, port);

    QUdpSocket* socket = qobject_cast<QUdpSocket*>(connection.socket());
    QSignalSpy spySocketHostFound(socket, SIGNAL(hostFound()));
    QSignalSpy spySocketConnected(socket, SIGNAL(connected()));
    QSignalSpy spySocketError(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    //QSignalSpy spySocketStateChanged(socket , SIGNAL(stateChanged(QAbstractSocket::SocketState)));

    QCOMPARE(connection.state(), QCoapConnection::UNCONNECTED);

    connection.connectToHostForTest();

    if (qstrcmp(QTest::currentDataTag(), "success") == 0) {
        QTRY_COMPARE_WITH_TIMEOUT(spySocketHostFound.count(), 1, 5000);
        QTRY_COMPARE_WITH_TIMEOUT(spySocketConnected.count(), 1, 5000);
        //QTRY_COMPARE_WITH_TIMEOUT(spySocketStateChanged.count(), 1, 5000);
        QCOMPARE(connection.state(), QCoapConnection::CONNECTED);
    }
    else {
        QTRY_COMPARE_WITH_TIMEOUT(spySocketError.count(), 1, 5000);
        //QTRY_COMPARE_WITH_TIMEOUT(spySocketStateChanged.count(), 0, 5000);
        QCOMPARE(connection.state(), QCoapConnection::UNCONNECTED);
    }
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<int>("port");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QString>("dataHexaHeader");
    QTest::addColumn<QString>("dataHexaPayload");

    QTest::newRow("simple_get_request") << "coap://" << "172.17.0.3" << "/test" << 5683 << QCoapRequest::GET << "5445" << "48656c6c6f20746573740a";
    // TODO : change reply to match (problem with the server)
    QTest::newRow("simple_put_request") << "coap://" << "172.17.0.3" << "/test" << 5683 << QCoapRequest::GET << "5445" << "48656c6c6f20746573740a";
    QTest::newRow("simple_post_request") << "coap://" << "172.17.0.3" << "/test" << 5683 << QCoapRequest::GET << "5445" << "48656c6c6f20746573740a";
    QTest::newRow("simple_delete_request") << "coap://" << "172.17.0.3" << "/test" << 5683 << QCoapRequest::GET << "5445" << "48656c6c6f20746573740a";
}

void tst_QCoapConnection::sendRequest()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(int, port);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QString, dataHexaHeader);
    QFETCH(QString, dataHexaPayload);

    QCoapConnectionForTest connection(host, port);

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(&connection, SIGNAL(readyRead()));

    QCoapRequest request(protocol + host + path);
    request.setMessageId(request.generateMessageId());
    request.setToken(QByteArray("abcd"));
    request.setOperation(operation);
    QVERIFY(connection.socket() != nullptr);
    connection.sendRequest(request.toPdu());

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 10000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 10000);

    QByteArray reply = connection.readReply();

    QVERIFY(QString(reply.toHex()).startsWith(dataHexaHeader));
    QVERIFY(QString(reply.toHex()).contains(dataHexaPayload));
}

void tst_QCoapConnection::writeToSocket_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("success") << "Data for the writing/reading to a socket(QIODevice) test";
}

void tst_QCoapConnection::writeToSocket()
{
    QFETCH(QString, data);

    QCoapConnectionForTest connection;
    QBuffer* socket = new QBuffer();

    connection.setSocketForTest(socket);
    connection.writeToSocketForTests(data.toUtf8());

    QCOMPARE(connection.readReply(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
