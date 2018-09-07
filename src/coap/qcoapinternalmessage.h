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

#ifndef QCOAPINTERNALMESSAGE_H
#define QCOAPINTERNALMESSAGE_H

#include <QtCoap/qcoapmessage.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QCoapParser;

class QCoapInternalMessagePrivate;
class Q_AUTOTEST_EXPORT QCoapInternalMessage : public QObject
{
    Q_OBJECT
public:
    explicit QCoapInternalMessage(QObject *parent = nullptr);
    explicit QCoapInternalMessage(const QCoapMessage &message, QObject *parent = nullptr);
    QCoapInternalMessage(const QCoapInternalMessage &other, QObject *parent = nullptr);
    virtual ~QCoapInternalMessage() {}

    static void setParser(const QCoapParser*);

    void addOption(QCoapOption::OptionName name, const QByteArray &value);
    void addOption(QCoapOption::OptionName name, quint32 value);
    virtual void addOption(const QCoapOption &option);
    void removeOption(QCoapOption::OptionName name);

    QCoapMessage *message();
    const QCoapMessage *message() const;

    uint currentBlockNumber() const;
    bool hasMoreBlocksToReceive() const;
    uint blockSize() const;
    void setCurrentBlockNumber(uint);
    void setHasNextBlock(bool);
    void setBlockSize(uint);

    virtual bool isValid() const;
    static bool isUrlValid(const QUrl &url);

protected:
    explicit QCoapInternalMessage(QCoapInternalMessagePrivate &dd, QObject *parent = nullptr);

    void updateFromDescriptiveBlockOption(const QCoapOption &option);

    Q_DECLARE_PRIVATE(QCoapInternalMessage)
};

QT_END_NAMESPACE

#endif // QCOAPINTERNALMESSAGE_H
