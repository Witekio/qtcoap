#include <QtTest>
#include <QCoreApplication>

#include "qcoapresource.h"
#include "qcoapprotocol.h"

class tst_QCoapResource : public QObject
{
    Q_OBJECT

public:
    tst_QCoapResource();
    ~tst_QCoapResource();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void parseCoreLink_data();
    void parseCoreLink();
};

tst_QCoapResource::tst_QCoapResource()
{
}

tst_QCoapResource::~tst_QCoapResource()
{
}

void tst_QCoapResource::initTestCase()
{
}

void tst_QCoapResource::cleanupTestCase()
{
}

void tst_QCoapResource::parseCoreLink_data()
{
    QTest::addColumn<int>("resourceNumber");
    QTest::addColumn<QList<QString>>("pathList");
    QTest::addColumn<QList<QString>>("titleList");
    QTest::addColumn<QList<QString>>("resourceTypeList");
    QTest::addColumn<QList<uint>>("contentFormatList");
    QTest::addColumn<QList<QString>>("interfaceList");
    QTest::addColumn<QList<int>>("maximumSizeList");
    QTest::addColumn<QList<bool>>("observableList");
    QTest::addColumn<QByteArray>("coreLinkList");

    QList<QString> pathList;
    pathList << "/obs" << "/separate" << "/seg1" << "/seg1/seg2" << "/large-separate"
             << "/.well-known/core" << "/multi-format" << "/path"
             << "/path/sub1" << "/link1" << "/validate" << "/test"
             << "/query" << "/large-post" << "/obs-non" << "/shutdown";

    QList<QString> titleList;
    titleList << "Observable resource which changes every 5 seconds"
              << "Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way"
              << "Long path resource"
              << "Long path resource"
              << "Large resource"
              << ""
              << "Resource that exists in different content formats (text/plain utf8 and application/xml)"
              << "Hierarchical link description entry"
              << "Hierarchical link description sub-resource"
              << "Link test resource"
              << "Resource which varies"
              << "Default test resource"
              << "Resource accepting query parameters"
              << "Handle PostOperation with two-way blockwise transfer"
              << "Observable resource which changes every 5 seconds"
              << "";

    QList<QString> resourceTypeList;
    resourceTypeList << "observe" << "" << "" << "" << "block" << "" << "" << "" << ""
                     << "Type1 Type2" << "" << "" << "" << "block" << "observe" << "";

    QList<uint> contentFormatList;
    contentFormatList << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 40 << 0 << 0 << 0 << 0
                      << 0 << 0 << 0 << 0;

    QList<QString> interfaceList;
    interfaceList << "" << "" << "" << "" << "" << "" << "" << "" << "" << "If1"
                  << "" << "" << "" << "" << "" << "";

    QList<int> maximumSizeList;
    maximumSizeList << -1 << -1 << -1 << -1 << 1280  << -1 << -1 << -1 << -1 << -1
                    << -1 << -1 << -1 << -1 << -1 << -1;

    QList<bool> observableList;
    observableList << true << false << false << false << false << false << false
                   << false << false << false << false << false << false << false
                   << true << false;

    QTest::newRow("parse") << 16
                           << pathList
                           << titleList
                           << resourceTypeList
                           << contentFormatList
                           << interfaceList
                           << maximumSizeList
                           << observableList
                           << QByteArray("</obs>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</separate>;title=\"Resource which cannot be served immediately and which cannot be acknowledged in a piggy-backed way\",</seg1>;title=\"Long path resource\",</seg1/seg2>;title=\"Long path resource\",</large-separate>;rt=\"block\";sz=1280;title=\"Large resource\",</.well-known/core>,</multi-format>;ct=\"0 41\";title=\"Resource that exists in different content formats (text/plain utf8 and application/xml)\",</path>;ct=40;title=\"Hierarchical link description entry\",</path/sub1>;title=\"Hierarchical link description sub-resource\",</link1>;if=\"If1\";rt=\"Type1 Type2\";title=\"Link test resource\",</validate>;title=\"Resource which varies\",</test>;title=\"Default test resource\",</query>;title=\"Resource accepting query parameters\",</large-post>;rt=\"block\";title=\"Handle PostOperation with two-way blockwise transfer\",</obs-non>;obs;rt=\"observe\";title=\"Observable resource which changes every 5 seconds\",</shutdown>");
}

void tst_QCoapResource::parseCoreLink()
{
    QFETCH(int, resourceNumber);
    QFETCH(QList<QString>, pathList);
    QFETCH(QList<QString>, titleList);
    QFETCH(QList<QString>, resourceTypeList);
    QFETCH(QList<uint>, contentFormatList);
    QFETCH(QList<QString>, interfaceList);
    QFETCH(QList<int>, maximumSizeList);
    QFETCH(QList<bool>, observableList);
    QFETCH(QByteArray, coreLinkList);

    QList<QCoapResource> resourceList;
    resourceList = QCoapProtocol::resourcesFromCoreLinkList(coreLinkList);

    QCOMPARE(resourceList.size(), resourceNumber);
    for (int i = 0; i < resourceList.size(); ++i) {
        QCOMPARE(resourceList[i].path(), pathList[i]);
        QCOMPARE(resourceList[i].title(), titleList[i]);
        QCOMPARE(resourceList[i].resourceType(), resourceTypeList[i]);
        QCOMPARE(resourceList[i].contentFormat(), contentFormatList[i]);
        QCOMPARE(resourceList[i].interface(), interfaceList[i]);
        QCOMPARE(resourceList[i].maximumSize(), maximumSizeList[i]);
        QCOMPARE(resourceList[i].observable(), observableList[i]);
    }
}

QTEST_MAIN(tst_QCoapResource)

#include "tst_qcoapresource.moc"
