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
    QTest::addColumn<QCoapReply::QCoapReplyStatusCode>("statusCode");
    QTest::addColumn<QCoapMessage::QCoapMessageType>("type");
    QTest::addColumn<qint16>("messageId");
    QTest::addColumn<qint64>("token");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pdu");

    QTest::newRow("reply") << QCoapReply::CONTENT
                           << QCoapReply::NONCONFIRMABLE
                           << qint16(64463)
                           << qint64(0x4647f09b)
                           << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
                           << "5445fbcf4647f09bc0211eff547970653a203120284e4f4e290a436f64653a20312028474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";
}

void tst_QCoapReply::parseReplyPdu()
{
    QFETCH(QCoapReply::QCoapReplyStatusCode, statusCode);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(qint16, messageId);
    QFETCH(qint64, token);
    QFETCH(QString, payload);
    QFETCH(QString, pdu);

    QCoapReply reply;
    reply.fromPdu(pdu.toUtf8());

    QCOMPARE(reply.statusCode(), statusCode);
    QCOMPARE(reply.type(), type);
    QCOMPARE(reply.token(), token);
    QCOMPARE(reply.messageId(), messageId);
    QCOMPARE(reply.payload(), payload);
}

QTEST_MAIN(tst_QCoapReply)

#include "tst_qcoapreply.moc"
