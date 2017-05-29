#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <qcoaprequest.h>
#include <qcoapconnection.h>

Q_DECLARE_METATYPE(QCoapRequest::QCoapRequestOperation)
Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType)

class tst_QCoapRequest : public QObject
{
    Q_OBJECT

public:
    tst_QCoapRequest();
    ~tst_QCoapRequest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void ctor_data();
    void ctor();
    void setUrl_data();
    void setUrl();
    void setOperation_data();
    void setOperation();
    void tokenGeneration();
    void messageIdGeneration();
    void requestToPdu_data();
    void requestToPdu();
    void sendRequest_data();
    void sendRequest();
    void updateReply_data();
    void updateReply();
};

class QCoapRequestForTests : public QCoapRequest {
public:
    QCoapRequestForTests(const QUrl& url = QUrl()) :
        QCoapRequest(url)
    {
        connection_p = nullptr;
        reply_p = nullptr;
    }

    void setConnection(QCoapConnection* connection) {
        connection_p = connection;
    }

    void setReply(QCoapReply* reply) {
        reply_p = reply;
    }

    QCoapConnection* connection() const { return connection_p; }
    QCoapReply* reply() const { return reply_p; }
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

void tst_QCoapRequest::ctor_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("empty") << QUrl();
    QTest::newRow("coap") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
}

void tst_QCoapRequest::ctor()
{
    QFETCH(QUrl, url);

    QCoapRequestForTests request(url);
    QCOMPARE(request.url(), url);
    QVERIFY(request.reply() != nullptr);
    QVERIFY(request.connection() != nullptr);
}

void tst_QCoapRequest::setUrl_data()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("empty") << QUrl();
    QTest::newRow("coap") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
}

void tst_QCoapRequest::setUrl()
{
    QFETCH(QUrl, url);

    QCoapRequest request;
    request.setUrl(url);
    QCOMPARE(request.url(), url);
}

void tst_QCoapRequest::setOperation_data()
{
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");

    QTest::newRow("get") << QCoapRequest::GET;
    QTest::newRow("put") << QCoapRequest::PUT;
    QTest::newRow("post") << QCoapRequest::POST;
    QTest::newRow("delete") << QCoapRequest::DELETE;
    QTest::newRow("other") << QCoapRequest::OTHER;
}

void tst_QCoapRequest::setOperation()
{
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);

    QCoapRequest request;
    request.setOperation(operation);
    QCOMPARE(request.operation(), operation);
}

void tst_QCoapRequest::tokenGeneration()
{
    QList<qint64> tokenList;

    QCoapRequest request;

    for (int i = 0; i < 500; ++i) {
        qint64 token = request.generateToken();
        QVERIFY(!tokenList.contains(token));
        tokenList.push_back(token);
    }
}

void tst_QCoapRequest::messageIdGeneration()
{
    QCoapRequest request;
    qint64 firstId = request.generateMessageId();

    for (int i = 1; i <= 500; ++i) {
        qint16 id = request.generateMessageId();
        QVERIFY(id == (firstId + i));
    }
}

void tst_QCoapRequest::requestToPdu_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<QString>("pduHeader");
    QTest::addColumn<QString>("pduOptions");
    QTest::addColumn<QString>("pduPayload");

    QTest::newRow("request") << QUrl("coap://vs0.inf.ethz.ch:5683/test") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << "5401" << "b474657374" << "";
}

void tst_QCoapRequest::requestToPdu()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(QString, pduHeader);
    QFETCH(QString, pduOptions);
    QFETCH(QString, pduPayload);

    QCoapRequest request(url);

    request.setType(type);
    request.setOperation(operation);
    qint16 id = request.generateMessageId();
    qint64 token = request.generateToken();

    QByteArray pdu;
    pdu.append(pduHeader);
    pdu.append(QString::number(id, 16).toUtf8());
    pdu.append(QString::number(token, 16).toUtf8());
    pdu.append(pduOptions);
    pdu.append(pduPayload);

    QCOMPARE(request.toPdu(), pdu);
}

void tst_QCoapRequest::sendRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");

    QTest::newRow("get_nonconfirmable") << QUrl("coap://vs0.inf.ethz.ch:5683/test") << QCoapRequest::GET << QCoapMessage::NONCONFIRMABLE;
}

void tst_QCoapRequest::sendRequest()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QCoapMessage::QCoapMessageType, type);

    QCoapRequestForTests request(url);

    request.setType(type);
    request.setOperation(operation);

    QSignalSpy spyConnectionReadyRead(request.connection(), SIGNAL(readyRead()));
    request.sendRequest();

    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 5000);
}

class QCoapReplyForTests : public QCoapReply
{
public:
    QCoapReplyForTests() : QCoapReply() {}

    void fromPdu(const QByteArray& pdu) {
        payload_p = pdu;
    }
};

class QCoapConnectionForTests : public QCoapConnection
{
public:
    QCoapConnectionForTests(const QString& host = "localhost", int port = 5683, QObject* parent = nullptr) :
        QCoapConnection(host, port, parent)
    {}

    void setDataReply(const QByteArray& newDataReply) {
        dataReply = newDataReply;
    }

    QByteArray readReply() {
        return dataReply;
    }

private:
    QByteArray dataReply;
};

void tst_QCoapRequest::updateReply_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("success") << "Data for the reading test";
}

void tst_QCoapRequest::updateReply()
{
    QFETCH(QString, data);

    QCoapRequestForTests request;
    QCoapConnectionForTests connection;

    QSignalSpy spyConnectionFinished(&request, SIGNAL(finished()));

    connection.setDataReply(data.toUtf8());
    request.setConnection(&connection);

    request.readReply();
    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionFinished.count(), 1, 5000);
    QCOMPARE(request.reply()->payload(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
