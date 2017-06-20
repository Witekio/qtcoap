#ifndef QCOAPREQUEST_P_H
#define QCOAPREQUEST_P_H

#include "qcoaprequest.h"
#include "qcoapmessage_p.h"

QT_BEGIN_NAMESPACE

class QCoapRequestPrivate : public QCoapMessagePrivate
{
public:
    QCoapRequestPrivate();
    QCoapRequestPrivate(const QCoapRequestPrivate &other);
    ~QCoapRequestPrivate();

    QUrl url;
    QCoapConnection* connection;
    QCoapProtocol* protocol;
    QCoapReply* reply;
    QCoapRequest::QCoapRequestOperation operation;
    QCoapRequest::QCoapRequestState state;
    uint blockAsked;
    bool observe;

    //void _q_startToSend();
    //void _q_readReply();
    //void _q_getNextBlock(uint blockAsked);
    //void _q_sendAck(quint16 messageId);
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_P_H
