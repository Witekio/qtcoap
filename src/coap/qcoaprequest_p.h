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
    QCoapOperation operation;
    QCoapRequest::QCoapRequestState state;
    bool observe;
};

QT_END_NAMESPACE

#endif // QCOAPREQUEST_P_H
