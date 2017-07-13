#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <QUdpSocket>
#include <QBuffer>
#include <QtCore/qglobal.h>
#include "qcoapglobal.h"
#include "qcoapconnection.h"
#include "qcoapconnection_p.h"
#include "qcoaprequest.h"
#include "qcoaprequest_p.h"

Q_DECLARE_METATYPE(QCoapOperation)

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
};

class QCoapConnectionForTest : public QCoapConnection
{
    Q_OBJECT
public:
    QCoapConnectionForTest(const QString& host = "localhost",
                           quint16 port = 5683,
                           QObject* parent = nullptr) :
        QCoapConnection(host, port, parent)
    {}

    //TODO test bound instead void connectToHostForTest() { connectToHost(); }
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
    QTest::addColumn<quint16>("port");

    QTest::newRow("hostAndPort") << "test-server" << quint16(5684);
    QTest::newRow("hostOnly") << "test-server" << quint16(5683);
    QTest::newRow("nothing") << "localhost" << quint16(5683);
}

void tst_QCoapConnection::ctor()
{
    QFETCH(QString, host);
    QFETCH(quint16, port);

    QCoapConnection* connection = nullptr;

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
    QTest::addColumn<quint16>("port");

    QTest::newRow("success") << "coap://" << "vs0.inf.ethz.ch" << "/test" << quint16(5683);
    QTest::newRow("failure") << "coap://" << "not-a-test-server" << "/will-fail" << quint16(5683);
}

void tst_QCoapConnection::connectToHost()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(quint16, port);

    QCoapConnectionForTest connection(host, port);

    QUdpSocket* socket = qobject_cast<QUdpSocket*>(connection.socket());
    QSignalSpy spySocketHostFound(socket, SIGNAL(hostFound()));
    QSignalSpy spySocketConnected(socket, SIGNAL(connected()));
    QSignalSpy spySocketError(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    //QSignalSpy spySocketStateChanged(socket , SIGNAL(stateChanged(QAbstractSocket::SocketState)));

    QCOMPARE(connection.state(), QCoapConnection::Unconnected);

    // TODO : test bind
    /*connection.connectToHostForTest();

    if (qstrcmp(QTest::currentDataTag(), "success") == 0) {
        QTRY_COMPARE_WITH_TIMEOUT(spySocketHostFound.count(), 1, 5000);
        QTRY_COMPARE_WITH_TIMEOUT(spySocketConnected.count(), 1, 5000);
        //QTRY_COMPARE_WITH_TIMEOUT(spySocketStateChanged.count(), 1, 5000);
        QCOMPARE(connection.state(), QCoapConnection::Connected);
    }
    else {
        QTRY_COMPARE_WITH_TIMEOUT(spySocketError.count(), 1, 5000);
        //QTRY_COMPARE_WITH_TIMEOUT(spySocketStateChanged.count(), 0, 5000);
        QCOMPARE(connection.state(), QCoapConnection::Unconnected);
    }*/
}

void tst_QCoapConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<quint16>("port");
    QTest::addColumn<QCoapOperation>("operation");
    QTest::addColumn<QString>("dataHexaHeader");
    QTest::addColumn<QString>("dataHexaPayload");

    QTest::newRow("simple_get_request") << "coap://" << "172.17.0.3" << "/test" << quint16(5683) << GetCoapOperation << "5445" << "61626364c0211eff547970653a203120284e4f4e290a436f64653a20312028474554290a4d49443a2032343830360a546f6b656e3a203631363236333634";
    QTest::newRow("simple_put_request") << "coap://" << "172.17.0.3" << "/test" << quint16(5683) << PutCoapOperation << "5444" << "61626364";
    QTest::newRow("simple_post_request") << "coap://" << "172.17.0.3" << "/test" << quint16(5683) << PostCoapOperation << "5441" << "61626364896c6f636174696f6e31096c6f636174696f6e32096c6f636174696f6e33";
    QTest::newRow("simple_delete_request") << "coap://" << "172.17.0.3" << "/test" << quint16(5683) << DeleteCoapOperation << "5442" << "61626364";
}

void tst_QCoapConnection::sendRequest()
{
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(quint16, port);
    QFETCH(QCoapOperation, operation);
    QFETCH(QString, dataHexaHeader);
    QFETCH(QString, dataHexaPayload);

    QCoapConnectionForTest connection(host, port);

    QSignalSpy spySocketReadyRead(connection.socket(), SIGNAL(readyRead()));
    QSignalSpy spyConnectionReadyRead(&connection, SIGNAL(readyRead(const QByteArray&)));

    QCoapRequest request(protocol + host + path);
    request.setMessageId(24806);
    request.setToken(QByteArray("abcd"));
    request.setOperation(operation);
    QVERIFY(connection.socket() != nullptr);
    QCoapInternalRequest internalRequest(request);
    connection.sendRequest(internalRequest.toQByteArray(), host, port);

    QTRY_COMPARE_WITH_TIMEOUT(spySocketReadyRead.count(), 1, 5000);
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 5000);

    QByteArray reply = connection.readAll();

    QVERIFY(QString(reply.toHex()).startsWith(dataHexaHeader));
    QVERIFY(QString(reply.toHex()).endsWith(dataHexaPayload));
}

QTEST_MAIN(tst_QCoapConnection)

#include "tst_qcoapconnection.moc"
