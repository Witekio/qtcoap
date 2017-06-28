#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include "qcoapprotocol.h"
#include "private/qobject_p.h"
#include <QList>
#include <QPair>
#include <QQueue>

QT_BEGIN_NAMESPACE

//typedef QMap<QCoapInternalRequest, QList<QCoapInternalReply>> InternalMessageMap;
typedef QMap<QCoapReply*, QPair<QCoapInternalRequest, QList<QCoapInternalReply>>> InternalMessageMap;

class QCoapProtocolPrivate : public QObjectPrivate
{
public:
    enum ProtocolState {
        WAITING,
        SENDING
    };

    QCoapProtocolPrivate();

    void handleFrame();

    void setState(ProtocolState newState);
    void sendRequest(const QCoapInternalRequest& request);
    bool containsMessageId(quint16 id);
    bool containsToken(const QByteArray& token);
    //QCoapInternalRequest findInternalRequestByToken(const QByteArray& token);
    QCoapReply* findReplyByToken(const QByteArray& token);

    InternalMessageMap internalReplies;
    QQueue<QByteArray> frameQueue;
    ProtocolState state;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

#endif // QCOAPPROTOCOL_P_H
