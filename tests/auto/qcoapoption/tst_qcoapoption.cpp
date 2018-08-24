/****************************************************************************
**
** Copyright (C) 2018 Witekio.
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

#include <QtCoap/qcoapoption.h>

class tst_QCoapOption : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructWithQByteArray();
    void constructWithQStringView();
    void constructWithCString();
    void constructWithInteger();
    void constructWithUtf8Characters();
};

void tst_QCoapOption::constructWithQByteArray()
{
    QByteArray ba = "some data";
    QCoapOption option(QCoapOption::LocationPath, ba);

    QCOMPARE(option.value(), ba);
}

void tst_QCoapOption::constructWithQStringView()
{
    QString str = "some data";
    QCoapOption option(QCoapOption::LocationPath, str);

    QCOMPARE(option.value(), str.toUtf8());
}

void tst_QCoapOption::constructWithCString()
{
    const char *str = "some data";
    QCoapOption option(QCoapOption::LocationPath, str);

    QCOMPARE(option.value(), QByteArray(str));
}

void tst_QCoapOption::constructWithInteger()
{
    quint32 value = 64000;
    QCoapOption option(QCoapOption::Size1, value);

    QCOMPARE(option.valueToInt(), value);
}

void tst_QCoapOption::constructWithUtf8Characters()
{
    QByteArray ba = "\u00E9~\u03BB\u20B2";
    QCoapOption option(QCoapOption::LocationPath, ba);

    QCOMPARE(option.value(), ba);
}

QTEST_APPLESS_MAIN(tst_QCoapOption)

#include "tst_qcoapoption.moc"
