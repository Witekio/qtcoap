#include <QtTest>
#include <QCoreApplication>

// add necessary includes here
#include "qcoapreply.h"

Q_DECLARE_METATYPE(QCoapReply::QCoapReplyStatusCode);
Q_DECLARE_METATYPE(QCoapMessage::QCoapMessageType);
Q_DECLARE_METATYPE(QCoapOption::QCoapOptionName);

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
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<quint8>("tokenLength");
    QTest::addColumn<int>("optionsListLength");
    QTest::addColumn<QList<QCoapOption::QCoapOptionName>>("optionsNames");
    QTest::addColumn<QList<quint8>>("optionsLengths");
    QTest::addColumn<QList<QByteArray>>("optionsValues");
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QString>("pduHexa");

    QList<QCoapOption::QCoapOptionName> optionsNamesReply({QCoapOption::CONTENTFORMAT,
                                                          QCoapOption::MAXAGE});
    QList<quint8> optionsLengthsReply({0,1});
    QList<QByteArray> optionsValuesReply({"", QByteArray::fromHex("1e")});

    QList<QCoapOption::QCoapOptionName> bigOptionsNamesReply({QCoapOption::SIZE1});
    QList<quint8> bigOptionsLengthsReply({26});
    QList<QByteArray> bigOptionsValuesReply({QByteArray("abcdefghijklmnopqrstuvwxyz")});

    QTest::newRow("reply_with_option_and_payload") << QCoapReply::CONTENT
                                                   << QCoapReply::NONCONFIRMABLE
                                                   << quint16(64463)
                                                   << QByteArray("4647f09b")
                                                   << quint8(4)
                                                   << 2
                                                   << optionsNamesReply
                                                   << optionsLengthsReply
                                                   << optionsValuesReply
                                                   << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
                                                   << "5445fbcf4647f09bc0211eff547970653a203120284e4f4e290a436f64653a20312028474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";

    QTest::newRow("reply_without_options") << QCoapReply::CONTENT
                                           << QCoapReply::NONCONFIRMABLE
                                           << quint16(64463)
                                           << QByteArray("4647f09b")
                                           << quint8(4)
                                           << 0
                                           << QList<QCoapOption::QCoapOptionName>()
                                           << QList<quint8>()
                                           << QList<QByteArray>()
                                           << "Type: 1 (NON)\nCode: 1 (GET)\nMID: 56400\nToken: 4647f09b"
                                           << "5445fbcf4647f09bff547970653a203120284e4f4e290a436f64653a20312028474554290a4d49443a2035363430300a546f6b656e3a203436343766303962";

    QTest::newRow("reply_without_payload") << QCoapReply::CONTENT
                                           << QCoapReply::NONCONFIRMABLE
                                           << quint16(64463)
                                           << QByteArray("4647f09b")
                                           << quint8(4)
                                           << 2
                                           << optionsNamesReply
                                           << optionsLengthsReply
                                           << optionsValuesReply
                                           << ""
                                           << "5445fbcf4647f09bc0211e";

    QTest::newRow("reply_only") << QCoapReply::CONTENT
                                << QCoapReply::NONCONFIRMABLE
                                << quint16(64463)
                                << QByteArray("4647f09b")
                                << quint8(4)
                                << 0
                                << QList<QCoapOption::QCoapOptionName>()
                                << QList<quint8>()
                                << QList<QByteArray>()
                                << ""
                                << "5445fbcf4647f09b";

    QTest::newRow("reply_with_big_option") << QCoapReply::CONTENT
                                           << QCoapReply::NONCONFIRMABLE
                                           << quint16(64463)
                                           << QByteArray("4647f09b")
                                           << quint8(4)
                                           << 1
                                           << bigOptionsNamesReply
                                           << bigOptionsLengthsReply
                                           << bigOptionsValuesReply
                                           << ""
                                           << "5445fbcf4647f09bdd2f0d6162636465666768696a6b6c6d6e6f707172737475767778797a";

}

void tst_QCoapReply::parseReplyPdu()
{
    QFETCH(QCoapReply::QCoapReplyStatusCode, statusCode);
    QFETCH(QCoapMessage::QCoapMessageType, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(quint8, tokenLength);
    QFETCH(int, optionsListLength);
    QFETCH(QList<QCoapOption::QCoapOptionName>, optionsNames);
    QFETCH(QList<quint8>, optionsLengths);
    QFETCH(QList<QByteArray>, optionsValues);
    QFETCH(QString, payload);
    QFETCH(QString, pduHexa);

    QCoapReply reply;
    reply.fromPdu(QByteArray::fromHex(pduHexa.toUtf8()));

    QCOMPARE(reply.type(), type);
    QCOMPARE(reply.tokenLength(), tokenLength);
    QCOMPARE(reply.statusCode(), statusCode);
    QCOMPARE(reply.messageId(), messageId);
    QCOMPARE(reply.token().toHex(), token);
    QCOMPARE(reply.optionsLength(), optionsListLength);
    for (int i = 0; i < optionsListLength; ++i) {
        QCoapOption* option = reply.option(i);
        qDebug() << option->name();
        QCOMPARE(option->name(), optionsNames.at(i));
        QCOMPARE(option->length(), optionsLengths.at(i));
        QCOMPARE(option->value(), optionsValues.at(i));
    }
    QCOMPARE(reply.payload(), payload);
}

QTEST_MAIN(tst_QCoapReply)

#include "tst_qcoapreply.moc"
