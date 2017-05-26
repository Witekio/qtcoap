#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapreply.h"

Q_DECLARE_METATYPE(QCoapReply::QCoapReplyStatusCode);
Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType);

class tst_QCoapReply : public QObject
{
    Q_OBJECT

public:
    tst_QCoapReply();
    ~tst_QCoapReply();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void parseReplyPdu_data();
    void parseReplyPdu();
};

tst_QCoapReply::tst_QCoapReply()
{

}

tst_QCoapReply::~tst_QCoapReply()
{

}

void tst_QCoapReply::initTestCase()
{

}

void tst_QCoapReply::cleanupTestCase()
{

}

void tst_QCoapReply::parseReplyPdu_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QCoapReply::QCoapReplyStatusCode>("statusCode");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pdu");

    // TODO : change the pdu and payload strings to something that we expect to match
    QTest::newRow("reply") << QUrl("coap://test-server/temperature") << QCoapReply::CONTENT << QCoapReply::NONCONFIRMABLE << "PAYLOAD TO CHANGE" << "PDU TO CHANGE";
}

void tst_QCoapReply::parseReplyPdu()
{
    QFETCH(QUrl, url);
    QFETCH(QCoapReply::QCoapReplyStatusCode, statusCode);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(QString, payload);
    QFETCH(QString, pdu);

    QCoapReply reply;
    reply.fromPdu(pdu.toUtf8());

    QCOMPARE(reply.statusCode(), statusCode);
    QCOMPARE(reply.type(), type);
    QCOMPARE(reply.payload(), payload);
    // TODO : do other QCOMPARE()
}
QTEST_MAIN(tst_QCoapReply)

#include "tst_qcoapreply.moc"
