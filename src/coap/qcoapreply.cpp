#include "qcoapreply.h"
#include "qcoapreply_p.h"
#include <QDebug>
#include <QtMath>

QCoapReplyPrivate::QCoapReplyPrivate() :
    status(InvalidCode),
    message(QCoapMessage()),
    isRunning(false),
    isFinished(false)
{
}

QCoapReply::QCoapReply(QObject* parent) :
    QCoapReply(* new QCoapReplyPrivate, parent)
{
}

QCoapReply::QCoapReply(QCoapReplyPrivate &dd, QObject* parent) :
    QIODevice(dd, parent)
{
    open(QIODevice::ReadOnly);
}

QCoapReply::~QCoapReply()
{
    qDebug() << "~QCoapReply()";
    /*deleteLater();
    emit abortRequest(this);*/
}

// TODO : set isRunning to true when the request is launched

qint64 QCoapReply::readData(char* data, qint64 maxSize)
{
    QByteArray payload = d_func()->message.payload();

    qint64 len = qMin(maxSize, qint64(payload.size()) - pos());
    if (len <= 0)
        return qint64(0);
    memcpy(data, payload.constData() + pos(), static_cast<size_t>(len));

    return len;
}

qint64 QCoapReply::writeData(const char* data, qint64 maxSize)
{
    // The user cannot write to the reply
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

QCoapStatusCode QCoapReply::statusCode() const
{
    return d_func()->status;
}

QCoapMessage QCoapReply::message() const
{
    return d_func()->message;
}

QCoapRequest QCoapReply::request() const
{
    return d_func()->request;
}

bool QCoapReply::isFinished() const
{
    return d_func()->isFinished;
}

QUrl QCoapReply::url() const
{
    return d_func()->request.url();
}

QCoapOperation QCoapReply::operation() const
{
    return d_func()->request.operation();
}

void QCoapReply::setRequest(const QCoapRequest& request)
{
    Q_D(QCoapReply);
    d->request = request;
}

void QCoapReply::setIsRunning(bool isRunning)
{
    Q_D(QCoapReply);
    d->isRunning = isRunning;
}

void QCoapReply::updateFromInternalReply(const QCoapInternalReply& internalReply)
{
    qDebug() << "QCoapReply::updateFromInternalReply()";
    d_func()->message.setPayload(internalReply.payload());
    d_func()->message.setType(internalReply.type());
    d_func()->message.setVersion(internalReply.version());
    d_func()->status = internalReply.statusCode();
    d_func()->isFinished = true;
    d_func()->isRunning = false;

    if (d_func()->status >= BadRequestCode)
        replyError(d_func()->status);
    if (d_func()->request.observe())
        emit notified(internalReply.payload());

    emit finished();
}

void QCoapReply::replyError(QCoapStatusCode error)
{
    QCoapNetworkError networkError;
    switch (error) {
    case BadRequestCode:
        networkError = BadRequestCoapError;
        break;
    default:
        networkError = UnknownCoapError;
    }

    emit error(networkError);
}

void QCoapReply::connectionError(QAbstractSocket::SocketError socketError)
{
    QCoapNetworkError networkError;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError :
        networkError = HostNotFoundCoapError;
        break;

    default:
        networkError = UnknownCoapError;
    }

    emit error(networkError);
}

