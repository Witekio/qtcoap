#include "qcoapreply.h"
#include "qcoapreply_p.h"
#include <QDebug>
#include <QtMath>

QT_BEGIN_NAMESPACE

QCoapReplyPrivate::QCoapReplyPrivate() :
    status(InvalidCoapCode),
    message(QCoapMessage()),
    isRunning(false),
    isFinished(false),
    isAborted(false)
{
}

/*!
    \class QCoapReply
    \brief The QCoapReply class holds the data of a coap reply.

    The QCoapReply contains data related to a request sent with the
    QCoapClient.

    The finished signal is emitted when the response is fully received.

    The notified signal is emitted when a resource send a notification in
    response of an observe request.

    \sa QCoapClient, QCoapRequest, QCoapDiscoveryReply
*/

/*!
    \enum QNetworkReply::NetworkError

    Indicates all possible error conditions found during the
    processing of the request.

    \value NoCoapError              no error condition.

    \value HostNotFoundCoapError    the remote host name was not
                                    found.

    \value BadRequestCoapError      the request was not recognized.

    \value AddressInUseCoapError    the address is already in use.

    \value TimeOutCoapError         the response did not arrive in time.

    \value UnknownCoapError         an unknown error was detected.

    \sa error()
*/

/*!
    \fn void QCoapReply::finished()

    This signal is emitted whenever the reply is finished.

    \sa QCoapClient::finished(), isFinished(), notified()
*/

/*!
    \fn void QCoapReply::notified(const QByteArray&)

    This signal is emitted whenever a notification is received from a resource
    for an observe request.

    Its parameter is a byte array containing the payload of the notification.

    \sa QCoapClient::finished(), isFinished(), finished()
*/

/*!
    \fn void QCoapReply::error(QCoapReply::QCoapNetworkError)

    This signal is emitted whenever an error is sent by the socket or the code
    received into the reply is an error code.

    Its parameter is the error received.
*/

/*!
    \fn void QCoapReply::aborted(QCoapReply*);

    This signal is emitted when the request is aborted or the reply is deleted.

    Its parameter is the reply object related to the aborted request.
*/

/*!
    Constructs a QCoapReply object and sets \a parent as the parent object.
*/
QCoapReply::QCoapReply(QObject* parent) :
    QCoapReply(* new QCoapReplyPrivate, parent)
{
}

/*!
    \internal
*/
QCoapReply::QCoapReply(QCoapReplyPrivate &dd, QObject* parent) :
    QIODevice(dd, parent)
{
    open(QIODevice::ReadOnly);
}

/*!
    Destroys the QCoapReply object and abort the request in case of the
    response has not been received yet.
*/
QCoapReply::~QCoapReply()
{
    abortRequest();
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::readData(char* data, qint64 maxSize)
{
    QByteArray payload = d_func()->message.payload();

    qint64 len = qMin(maxSize, qint64(payload.size()) - pos());
    if (len <= 0)
        return qint64(0);
    memcpy(data, payload.constData() + pos(), static_cast<size_t>(len));

    return len;
}

/*!
  \internal

  \overload
*/
qint64 QCoapReply::writeData(const char* data, qint64 maxSize)
{
    // The user cannot write to the reply
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

/*!
    Returns the status code of the request.
*/
QCoapStatusCode QCoapReply::statusCode() const
{
    return d_func()->status;
}

/*!
    Returns the contained message.
*/
QCoapMessage QCoapReply::message() const
{
    return d_func()->message;
}

/*!
    Returns the associated request.

    \sa setRequest()
*/
QCoapRequest QCoapReply::request() const
{
    return d_func()->request;
}

/*!
    Returns true if the request is finished.

    \sa finished()
*/
bool QCoapReply::isFinished() const
{
    return d_func()->isFinished;
}

/*!
    Returns true if the request is running.
*/
bool QCoapReply::isRunning() const
{
    return d_func()->isRunning;
}

/*!
    Returns true if the request has been aborted.
*/
bool QCoapReply::isAborted() const
{
    return d_func()->isAborted;
}

/*!
    Returns the target uri of the associated request.
*/
QUrl QCoapReply::url() const
{
    return d_func()->request.url();
}

/*!
    Returns the operation of the associated request.
*/
QCoapOperation QCoapReply::operation() const
{
    return d_func()->request.operation();
}

/*!
    Returns the error of the reply or NoCoapError if there is no error.
*/
QCoapReply::QCoapNetworkError QCoapReply::errorReceived() const
{
    return d_func()->error;
}

/*!
    Sets the request associated to the QCoapReply.

    \sa request()
*/
void QCoapReply::setRequest(const QCoapRequest& request)
{
    Q_D(QCoapReply);
    d->request = request;
}

/*!
    Sets the isRunning parameter.

    \sa isRunning()
*/
void QCoapReply::setIsRunning(bool isRunning)
{
    Q_D(QCoapReply);
    d->isRunning = isRunning;
}

/*!
    Sets the error parameter.

    \sa errorReceived()
*/
void QCoapReply::setError(QCoapNetworkError newError)
{
    Q_D(QCoapReply);
    if (d->error == newError)
        return;

    d->error = newError;
    emit error(d->error);
}

/*!
    \internal

    Update the QCoapReply object and its message with data of the internal reply
    \a internalReply.
*/
void QCoapReply::updateFromInternalReply(const QCoapInternalReply& internalReply)
{
    if (!d_func()->isAborted) {
        qDebug() << "QCoapReply::updateFromInternalReply()";
        QCoapMessage internalReplyMessage = internalReply.message();
        d_func()->message.setPayload(internalReplyMessage.payload());
        d_func()->message.setType(internalReplyMessage.type());
        d_func()->message.setVersion(internalReplyMessage.version());
        d_func()->status = internalReply.statusCode();
        d_func()->isFinished = true;
        d_func()->isRunning = false;

        if (d_func()->status >= BadRequestCoapCode)
            replyError(d_func()->status);

        if (d_func()->request.observe())
            emit notified(internalReplyMessage.payload());

        emit finished();
    }
}

/*!
    Aborts the operation immediately and emit the aborted signal
    if the request was not finished before.
*/
void QCoapReply::abortRequest()
{
    Q_D(QCoapReply);
    d->isAborted = true;
    if (!this->isFinished())
        emit aborted(this);
}

/*!
    \internal

    Map the reply status code \a errorCode to the related coap network error.
*/
void QCoapReply::replyError(QCoapStatusCode errorCode)
{
    QCoapNetworkError networkError;
    switch (errorCode) {
    case BadRequestCoapCode:
        networkError = BadRequestCoapError;
        break;
    case UnauthorizedCoapCode:
        networkError = UnauthorizedCoapError;
        break;
    case BadOptionCoapCode:
        networkError = BadOptionCoapError;
        break;
    case ForbiddenCoapCode:
        networkError = ForbiddenCoapError;
        break;
    case NotFoundCoapCode:
        networkError = NotFoundCoapError;
        break;
    case MethodNotAllowedCoapCode:
        networkError = MethodNotAllowedCoapError;
        break;
    case NotAcceptableCoapCode:
        networkError = NotAcceptableCoapError;
        break;
    case RequestEntityIncompleteCoapCode:
        networkError = RequestEntityIncompleteCoapError;
        break;
    case PreconditionFailedCoapCode:
        networkError = PreconditionFailedCoapError;
        break;
    case RequestEntityTooLargeCoapCode:
        networkError = RequestEntityTooLargeCoapError;
        break;
    case UnsupportedContentFormatCoapCode:
        networkError = UnsupportedContentFormatCoapError;
        break;
    case InternalServerErrorCoapCode:
        networkError = InternalServerErrorCoapError;
        break;
    case NotImplementedCoapCode:
        networkError = NotImplementedCoapError;
        break;
    case BadGatewayCoapCode:
        networkError = BadGatewayCoapError;
        break;
    case ServiceUnavailableCoapCode:
        networkError = ServiceUnavailableCoapError;
        break;
    case GatewayTimeoutCoapCode:
        networkError = GatewayTimeoutCoapError;
        break;
    case ProxyingNotSupportedCoapCode:
        networkError = ProxyingNotSupportedCoapError;
        break;
    default:
        networkError = UnknownCoapError;
    }

    setError(networkError);
}

/*!
    \internal

    Map the socket error \a socketError to the related coap network error.
*/
void QCoapReply::connectionError(QAbstractSocket::SocketError socketError)
{
    QCoapNetworkError networkError;
    switch (socketError) {
    case QAbstractSocket::HostNotFoundError :
        networkError = HostNotFoundCoapError;
        break;
    case QAbstractSocket::AddressInUseError :
        networkError = AddressInUseCoapError;
        break;
    default:
        networkError = UnknownCoapError;
    }

    setError(networkError);
}

QT_END_NAMESPACE
