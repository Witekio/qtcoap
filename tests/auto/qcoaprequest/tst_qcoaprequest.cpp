#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapglobal.h"
#include <qcoaprequest.h>
#include <qcoaprequest_p.h>
#include <qcoapconnection.h>
#include <qcoapconnection_p.h>

Q_DECLARE_METATYPE(QCoapOperation)
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
    void internalRequestToFrame_data();
    void internalRequestToFrame();
    void parseUri_data();
    void parseUri();
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

    QCoapRequest request(url);
    QCOMPARE(request.url(), url);
    //QVERIFY(request.reply() != nullptr);
    //QVERIFY(request.connection() != nullptr);
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
    QTest::addColumn<QCoapOperation>("operation");

    QTest::newRow("get") << GetOperation;
    QTest::newRow("put") << PutOperation;
    QTest::newRow("post") << PostOperation;
    QTest::newRow("delete") << DeleteOperation;
    QTest::newRow("other") << OtherOperation;
}

void tst_QCoapRequest::setOperation()
{
    QFETCH(QCoapOperation, operation);

    QCoapRequest request;
    request.setOperation(operation);
    QCOMPARE(request.operation(), operation);
}

void tst_QCoapRequest::internalRequestToFrame_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapOperation>("operation");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<QString>("pduHeader");
    QTest::addColumn<QString>("pduPayload");

    QTest::newRow("request_with_option_and_payload") << QUrl("coap://172.17.0.3:5683/test") << GetOperation << QCoapRequest::NonConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374ff" << "Some payload";
    QTest::newRow("request_without_payload") << QUrl("coap://172.17.0.3:5683/test") << GetOperation << QCoapRequest::NonConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374" << "";
    QTest::newRow("request_without_option") << QUrl("coap://172.17.0.3:5683/") << PutOperation << QCoapRequest::ConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "4403dc504647f09bff" << "Some payload";
    QTest::newRow("request_only") << QUrl("coap://172.17.0.3:5683/") << GetOperation << QCoapRequest::NonConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09b" << "";
    QTest::newRow("request_with_multiple_options") << QUrl("coap://172.17.0.3:5683/test/oui") << GetOperation << QCoapRequest::NonConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374036f7569" << "";
    QTest::newRow("request_with_big_option_number") << QUrl("coap://172.17.0.3:5683/test") << GetOperation << QCoapRequest::NonConfirmableMessage << quint16(56400) << QByteArray::fromHex("4647f09b") << "5401dc504647f09bb474657374dd240d6162636465666768696a6b6c6d6e6f707172737475767778797aff" << "Some payload";
}

void tst_QCoapRequest::internalRequestToFrame()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapOperation, operation);
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
        request.addOption(QCoapOption::Size1Option, QByteArray("abcdefghijklmnopqrstuvwxyz"));
    }

    QByteArray pdu;
    pdu.append(pduHeader);
    if (!pduPayload.isEmpty()) {
        pdu.append(pduPayload.toUtf8().toHex());
    }

    QCoapInternalRequest internalRequest(request);
    QCOMPARE(internalRequest.toQByteArray().toHex(), pdu);
}

void tst_QCoapRequest::parseUri_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<int>("optionsNumber");

    QTest::newRow("uri1") << QUrl("coap://vs0.inf.ethz.ch:5683/test/path1/?rd=25&nd=4")
                             << 5;
    QTest::newRow("uri2") << QUrl("coap://172.17.0.3/test/path1/?rd=25&nd=4")
                             << 4;
    QTest::newRow("uri3") << QUrl("coap://172.17.0.3:5684/test/path1")
                             << 3;
}

void tst_QCoapRequest::parseUri()
{
    QFETCH(QUrl, url);
    QFETCH(int, optionsNumber);

    QCoapRequest request(url);

    QCOMPARE(request.optionsLength(), optionsNumber);
}



QTEST_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
