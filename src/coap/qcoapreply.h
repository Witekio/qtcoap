#ifndef QCOAPREPLY_H
#define QCOAPREPLY_H

#include "qcoapmessage.h"
#include <QByteArray>
#include <QIODevice>
#include <QSharedPointer>
#include "qcoapinternalreply_p.h"

QT_BEGIN_NAMESPACE

class QCoapRequest;
class QCoapReplyPrivate;
class QCoapReply : public QIODevice
{
    Q_OBJECT
public:
    QCoapReply(QObject* parent = Q_NULLPTR);

    QCoapStatusCode statusCode() const;
    QCoapMessage message() const;
    bool isFinished() const;

public slots:
    void updateWithInternalReply(const QCoapInternalReply& pdu);

signals:
    void finished();

private :
    qint64 readData(char* data, qint64 maxSize) Q_DECL_OVERRIDE;
    qint64 writeData(const char* data, qint64 maxSize) Q_DECL_OVERRIDE;

    Q_DECLARE_PRIVATE(QCoapReply)
};

QT_END_NAMESPACE

#endif // QCOAPREPLY_H
