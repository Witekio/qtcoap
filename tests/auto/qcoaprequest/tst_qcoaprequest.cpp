/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Contact: https://witekio.com/contact/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest>
#include <QCoreApplication>

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoaprequest.h>
#include <QtCoap/qcoapconnection.h>
#include <private/qcoapinternalrequest_p.h>

class tst_QCoapRequest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void ctor_data();
    void ctor();
    void setUrl_data();
    void setUrl();
    void setMethod_data();
    void setMethod();
    void copyAndDetach();
    void internalRequestToFrame_data();
    void internalRequestToFrame();
    void parseUri_data();
    void parseUri();
};

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
}

void tst_QCoapRequest::setUrl_data()
{
    QTest::addColumn<QUrl>("inputUrl");
    QTest::addColumn<QUrl>("expectedUrl");

    QTest::newRow("empty") << QUrl() << QUrl();
    QTest::newRow("coap") << QUrl("coap://10.11.12.13:5683/test") << QUrl("coap://10.11.12.13:5683/test");
    QTest::newRow("other_port") << QUrl("coap://10.11.12.13:8888/test") << QUrl("coap://10.11.12.13:8888/test");
    QTest::newRow("no_port") << QUrl("coap://vs0.inf.ethz.ch/test") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
    QTest::newRow("no_scheme_no_port") << QUrl("vs0.inf.ethz.ch/test") << QUrl("coap://vs0.inf.ethz.ch:5683/test");
    QTest::newRow("incorrect_scheme") << QUrl("http://vs0.inf.ethz.ch:5683/test") << QUrl();
    QTest::newRow("invalid") << QUrl("-coap://vs0.inf.ethz.ch:5683/test") << QUrl();
}

void tst_QCoapRequest::setUrl()
{
    QFETCH(QUrl, inputUrl);
    QFETCH(QUrl, expectedUrl);

    QCoapRequest request;
    request.setUrl(inputUrl);
    QCOMPARE(request.url(), expectedUrl);
}

void tst_QCoapRequest::setMethod_data()
{
    QTest::addColumn<QtCoap::Method>("method");

    QTest::newRow("get") << QtCoap::Get;
    QTest::newRow("put") << QtCoap::Put;
    QTest::newRow("post") << QtCoap::Post;
    QTest::newRow("delete") << QtCoap::Delete;
    QTest::newRow("other") << QtCoap::Other;
}

void tst_QCoapRequest::setMethod()
{
    QFETCH(QtCoap::Method, method);

    QCoapRequest request;
    request.setMethod(method);
    QCOMPARE(request.method(), method);
}

void tst_QCoapRequest::copyAndDetach()
{
    QCoapRequest a;
    a.setMessageId(3);
    a.setPayload("payload");
    a.setToken("token");
    a.setType(QCoapMessage::Acknowledgement);
    a.setVersion(5);
    a.setMethod(QtCoap::Delete);
    QUrl testUrl("coap://url:500/resource");
    a.setUrl(testUrl);
    QUrl testProxyUrl("test://proxyurl");
    a.setProxyUrl(testProxyUrl);

    // Test the QCoapMessage copy
    QCoapMessage b(a);
    QVERIFY2(b.messageId() == 3, "Message not copied correctly");
    QVERIFY2(b.payload() == "payload", "Message not copied correctly");
    QVERIFY2(b.token() == "token", "Message not copied correctly");
    QVERIFY2(b.type() == QCoapMessage::Acknowledgement, "Message not copied correctly");
    QVERIFY2(b.version() == 5, "Message not copied correctly");

    // Test the QCoapRequest copy
    QCoapRequest c(a);
    QVERIFY2(c.method() == QtCoap::Delete, "Request not copied correctly");
    QVERIFY2(c.url() == testUrl, "Request not copied correctly");
    QVERIFY2(c.proxyUrl() == testProxyUrl, "Request not copied correctly");

    // Detach
    c.setMessageId(9);
    QCOMPARE(c.messageId(), 9);
    QCOMPARE(a.messageId(), 3);
}

void tst_QCoapRequest::internalRequestToFrame_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QtCoap::Method>("method");
    QTest::addColumn<QCoapMessage::MessageType>("type");
    QTest::addColumn<quint16>("messageId");
    QTest::addColumn<QByteArray>("token");
    QTest::addColumn<QString>("pduHeader");
    QTest::addColumn<QString>("pduPayload");

    QTest::newRow("request_with_option_and_payload")
        << QUrl("coap://172.17.0.3:5683/test")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374ff"
        << "Some payload";

    QTest::newRow("request_domain")
        << QUrl("coap://domain.com:5683/test")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09b3a646f6d61696e2e636f6d8474657374ff"
        << "Some payload";

    // TODO: Not passing yet, see QCoapInternalRequest::addUriHostOption
    /*QTest::newRow("request_ipv6")
        << QUrl("coap://[::ffff:ac11:3]:5683/test")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374ff"
        << "Some payload";*/

    QTest::newRow("request_without_payload")
        << QUrl("coap://172.17.0.3:5683/test")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374"
        << "";

    QTest::newRow("request_without_option")
        << QUrl("coap://172.17.0.3:5683/")
        << QtCoap::Put
        << QCoapRequest::Confirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "4403dc504647f09bff"
        << "Some payload";

    QTest::newRow("request_only")
        << QUrl("coap://172.17.0.3:5683/")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09b"
        << "";

    QTest::newRow("request_with_multiple_options")
        << QUrl("coap://172.17.0.3:5683/test/oui")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374036f7569"
        << "";

    QTest::newRow("request_with_big_option_number")
        << QUrl("coap://172.17.0.3:5683/test")
        << QtCoap::Get
        << QCoapRequest::NonConfirmable
        << quint16(56400)
        << QByteArray::fromHex("4647f09b")
        << "5401dc504647f09bb474657374dd240d6162636465666768696a6b6c6d6e6f70"
           "7172737475767778797aff"
        << "Some payload";
}

void tst_QCoapRequest::internalRequestToFrame()
{
    QFETCH(QUrl, url);
    QFETCH(QtCoap::Method, method);
    QFETCH(QCoapMessage::MessageType, type);
    QFETCH(quint16, messageId);
    QFETCH(QByteArray, token);
    QFETCH(QString, pduHeader);
    QFETCH(QString, pduPayload);

    QCoapRequest request(url);
    request.setType(type);
    request.setMethod(method);
    request.setPayload(pduPayload.toUtf8());
    request.setMessageId(messageId);
    request.setToken(token);
    if (qstrcmp(QTest::currentDataTag(), "request_with_big_option_number") == 0)
        request.addOption(QCoapOption::Size1, QByteArray("abcdefghijklmnopqrstuvwxyz"));

    QByteArray pdu;
    pdu.append(pduHeader);
    if (!pduPayload.isEmpty())
        pdu.append(pduPayload.toUtf8().toHex());

    QCoapInternalRequest internalRequest(request);
    QCOMPARE(internalRequest.toQByteArray().toHex(), pdu);
}

void tst_QCoapRequest::parseUri_data()
{
    qRegisterMetaType<QVector<QCoapOption>>();
    QTest::addColumn<QUrl>("uri");
    QTest::addColumn<QUrl>("proxyUri");
    QTest::addColumn<QVector<QCoapOption>>("options");

    QTest::newRow("port_path")
                        << QUrl("coap://172.17.0.3:5684/test/path1")
                        << QUrl()
                        << QVector<QCoapOption>({
                            QCoapOption(QCoapOption::UriPort, "5684"),
                            QCoapOption(QCoapOption::UriPath, "test"),
                            QCoapOption(QCoapOption::UriPath, "path1") });

    QTest::newRow("path_query")
                        << QUrl("coap://172.17.0.3/test/path1/?rd=25&nd=4")
                        << QUrl()
                        << QVector<QCoapOption>({
                            QCoapOption(QCoapOption::UriPath, "test"),
                            QCoapOption(QCoapOption::UriPath, "path1"),
                            QCoapOption(QCoapOption::UriQuery, "rd=25"),
                            QCoapOption(QCoapOption::UriQuery, "nd=4") });

    QTest::newRow("host_path_query")
                        << QUrl("coap://vs0.inf.ethz.ch:5683/test/path1/?rd=25&nd=4")
                        << QUrl()
                        << QVector<QCoapOption>({
                            QCoapOption(QCoapOption::UriHost, "vs0.inf.ethz.ch"),
                            QCoapOption(QCoapOption::UriPath, "test"),
                            QCoapOption(QCoapOption::UriPath, "path1"),
                            QCoapOption(QCoapOption::UriQuery, "rd=25"),
                            QCoapOption(QCoapOption::UriQuery, "nd=4") });

    QTest::newRow("proxy_url")
                        << QUrl("coap://vs0.inf.ethz.ch:5683/test/path1/?rd=25&nd=4")
                        << QUrl("coap://172.17.0.32/test:5684/othertest/path")
                        << QVector<QCoapOption>({
                            QCoapOption(QCoapOption::ProxyUri, "coap://172.17.0.32/test:5684/othertest/path") });
}

void tst_QCoapRequest::parseUri()
{
    QFETCH(QUrl, uri);
    QFETCH(QUrl, proxyUri);
    QFETCH(QVector<QCoapOption>, options);

    QCoapRequest request(uri, QCoapMessage::NonConfirmable, proxyUri);
    QCoapInternalRequest internalRequest(request);

    for (QCoapOption opt : options)
        QVERIFY2(internalRequest.message()->optionList().contains(opt), "Missing option");

    QCOMPARE(options.count(), internalRequest.message()->optionCount());
}

QTEST_APPLESS_MAIN(tst_QCoapRequest)

#include "tst_qcoaprequest.moc"
