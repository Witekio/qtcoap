#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include <qcoaprequest.h>
#include <qcoaprequest_p.h>
#include <qcoapconnection.h>
#include <qcoapconnection_p.h>
#include <qcoapreply.h>
#include <qcoapreply_p.h>

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
    void requestToPdu_data();
    void requestToPdu();
    void sendRequest_data();
    void sendRequest();
    void blockwiseReply_data();
    void blockwiseReply();
    void updateReply_data();
    void updateReply();
};

class QCoapRequestForTests : public QCoapRequest {
    Q_OBJECT
public:
    QCoapRequestForTests(const QUrl& url = QUrl()) :
        QCoapRequest(url)
    {
    }

    void setConnectionForTests(QCoapConnection* connection) {
        setConnection(connection);
    }

    void setReplyForTests(QCoapReply* reply) {
        setReply(reply);
    }

public slots:
    void readReplyForTest() {
        readReply();
    }
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

void tst_QCoapRequest::requestToPdu_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<QString>("pduHeader");
    QTest::addColumn<QString>("pduPayload");

    QTest::newRow("request_with_option_and_payload") << QUrl("coap://vs0.inf.ethz.ch:5683/test") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374ff" << "Some payload";
    QTest::newRow("request_without_payload") << QUrl("coap://vs0.inf.ethz.ch:5683/test") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374" << "";
    QTest::newRow("request_without_option") << QUrl("coap://vs0.inf.ethz.ch:5683/") << QCoapRequest::PUT << QCoapRequest::CONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "4403dc504647f09bff" << "Some payload";
    QTest::newRow("request_only") << QUrl("coap://vs0.inf.ethz.ch:5683/") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09b" << "";
    QTest::newRow("request_with_multiple_options") << QUrl("coap://vs0.inf.ethz.ch:5683/test/oui") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374036f7569" << "";
    QTest::newRow("request_with_big_option_number") << QUrl("coap://vs0.inf.ethz.ch:5683/test") << QCoapRequest::GET << QCoapRequest::NONCONFIRMABLE << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374dd240d6162636465666768696a6b6c6d6e6f707172737475767778797aff" << "Some payload";
}

void tst_QCoapRequest::requestToPdu()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(QString, pduHeader);
    QFETCH(QString, pduPayload);

    QCoapRequest request(url);

    request.setType(type);
    request.setOperation(operation);
    request.setPayload(pduPayload.toUtf8());
    request.setMessageId(messageId);
    request.setToken(token);
    if (qstrcmp(QTest::currentDataTag(), "request_with_big_option_number") == 0) {
        request.addOption(QCoapOption::SIZE1, QByteArray("abcdefghijklmnopqrstuvwxyz"));
    }

    QByteArray pdu;
    pdu.append(pduHeader);
    if (!pduPayload.isEmpty()) {
        pdu.append(pduPayload.toUtf8().toHex());
    }

    QCOMPARE(request.toPdu().toHex(), pdu);
}

void tst_QCoapRequest::sendRequest_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");

    QTest::newRow("get_nonconfirmable") << QUrl("coap://172.17.0.3:5683/test") << QCoapRequest::GET << QCoapMessage::NONCONFIRMABLE;
}

void tst_QCoapRequest::sendRequest()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QCoapMessage::QCoapMessageType, type);

    QCoapRequestForTests request(url);

    request.setType(type);
    request.setOperation(operation);
    request.setToken(QByteArray("abcd"));
    request.setMessageId(request.generateMessageId());

    QSignalSpy spyConnectionReadyRead(request.connection(), SIGNAL(readyRead()));
    request.sendRequest();

    QTRY_COMPARE_WITH_TIMEOUT(spyConnectionReadyRead.count(), 1, 10000);
}

void tst_QCoapRequest::blockwiseReply_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapRequest::QCoapRequestOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<QByteArray>("replyData");

    QByteArray data;
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 1 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 2 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 3 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 4 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");
    data.append("/-------------------------------------------------------------\\\n");
    data.append("|                 RESOURCE BLOCK NO. 5 OF 5                   |\n");
    data.append("|               [each line contains 64 bytes]                 |\n");
    data.append("\\-------------------------------------------------------------/\n");

    QTest::newRow("get_large") << QUrl("coap://172.17.0.3:5683/large")
                               << QCoapRequest::GET
                               << QCoapMessage::NONCONFIRMABLE
                               << data;
}

void tst_QCoapRequest::blockwiseReply()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapRequest::QCoapRequestOperation, operation);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(QByteArray, replyData);

    for (int i = 0; i < 10; ++i) {
        QCoapRequestForTests request(url);

        request.setType(type);
        request.setOperation(operation);
        request.setToken(request.generateToken());
        request.setMessageId(request.generateMessageId());

        request.sendRequest();

        QTRY_COMPARE_WITH_TIMEOUT(request.reply()->readData(), replyData, 10000);
    }
}

class QCoapReplyForTests : public QCoapReply
{
public:
    QCoapReplyForTests() : QCoapReply() {}

    void fromPdu(const QByteArray& pdu) {
        setPayload(pdu);
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
    QCoapReplyForTests* reply = new QCoapReplyForTests();

    QCoapConnectionForTests* connection = new QCoapConnectionForTests();

    QSignalSpy spyRequestFinished(&request, SIGNAL(finished(QCoapRequest*)));

    connection->setDataReply(data.toUtf8());
    request.setConnectionForTests(connection);
    request.setReplyForTests(reply);

    request.readReplyForTest();
    QTRY_COMPARE_WITH_TIMEOUT(spyRequestFinished.count(), 1, 1000);
    QCOMPARE(request.reply()->payload(), data.toUtf8());
}

QTEST_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
