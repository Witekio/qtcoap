#include "qcoapinternalrequest_p.h"
#include "qcoaprequest.h"

#include <QtMath>

QCoapInternalRequestPrivate::QCoapInternalRequestPrivate() :
    operation(EmptyOperation),
    isValid(true),
    cancelObserve(false),
    retransmissionCounter(0),
    timeout(0),
    timer(new QTimer)
{
}

QCoapInternalRequest::QCoapInternalRequest() :
    QCoapInternalMessage(*new QCoapInternalRequestPrivate)
{
    // TODO ? Transform into QObject
    // connect(d_func()->timer, SIGNAL(timeout()), this, SLOT(_q_timeout))
}

QCoapInternalRequest::QCoapInternalRequest(const QCoapRequest& request) :
    QCoapInternalRequest()
{
    QCoapInternalRequestPrivate* d = d_func();
    d->message->setVersion(request.version());
    d->message->setType(request.type());
    d->message->setMessageId(request.messageId());
    d->message->setToken(request.token());
    for (int i = 0; i < request.optionsLength(); ++i)
        d->message->addOption(request.option(i));
    d->message->setPayload(request.payload());
    d->operation = request.operation();
}

QCoapInternalRequest QCoapInternalRequest::fromQCoapRequest(const QCoapRequest& request)
{
    QCoapInternalRequest internalRequest(request);
    internalRequest.d_func()->fullPayload = request.payload();
    return internalRequest;
}

QCoapInternalRequest QCoapInternalRequest::createAcknowledgment(quint16 messageId,
                                                                const QByteArray& token)
{
    QCoapInternalRequest internalRequest;
    QCoapMessage* internalRequestMessage = internalRequest.d_func()->message;

    internalRequestMessage->setType(QCoapMessage::AcknowledgmentMessage);
    internalRequest.setOperation(EmptyOperation);
    internalRequestMessage->setMessageId(messageId);
    internalRequestMessage->setToken(token);
    internalRequestMessage->setPayload(QByteArray());
    internalRequestMessage->removeAllOptions();

    return internalRequest;
}

QCoapInternalRequest QCoapInternalRequest::createReset(quint16 messageId)
{
    QCoapInternalRequest internalRequest;
    QCoapMessage* internalRequestMessage = internalRequest.d_func()->message;

    internalRequestMessage->setType(QCoapMessage::ResetMessage);
    internalRequest.setOperation(EmptyOperation);
    internalRequestMessage->setMessageId(messageId);
    internalRequestMessage->setToken(QByteArray());
    internalRequestMessage->setPayload(QByteArray());
    internalRequestMessage->removeAllOptions();

    return internalRequest;
}

QCoapInternalRequest QCoapInternalRequest::invalidRequest()
{
    QCoapInternalRequest internalRequest;
    QCoapInternalRequestPrivate* d = internalRequest.d_func();
    d->isValid = false;
    return internalRequest;
}

QByteArray QCoapInternalRequest::toQByteArray() const
{
    QCoapInternalRequestPrivate* d = d_func();
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(d->message->version()) << 30)    // Coap version
            | (quint32(d->message->type()) << 28)                  // Message type
            | (quint32(d->message->token().length()) << 24)        // Token Length
            | (quint32(d->operation) << 16)                        // Operation type
            | (quint32(d->message->messageId()));                  // Message ID

    pdu.append(static_cast<char>(coapHeader >> 24));
    pdu.append(static_cast<char>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<char>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<char>(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d->message->token());

    // Insert Options
    if (!d->message->optionList().isEmpty()) {
        // Sort options by ascending order
        QList<QCoapOption> optionList = d->message->optionList();
        qSort(optionList.begin(), optionList.end(),
              [](const QCoapOption& a, const QCoapOption& b) -> bool {
            return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (QCoapOption option : optionList) {
            qDebug() << "option : " << option.name();
            quint8 optionPdu;

            quint16 optionDelta = static_cast<quint16>(option.name()) - lastOptionNumber;
            bool isOptionDeltaExtended = false;
            quint8 optionDeltaExtended = 0;

            quint16 optionLength = option.length();
            bool isOptionLengthExtended = false;
            quint8 optionLengthExtended = 0;

            // Delta value > 12 : special values
            if (optionDelta > 268) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 269);
                optionDelta = 14;
                isOptionDeltaExtended = true;
            } else if (optionDelta > 12) {
                optionDeltaExtended = static_cast<quint8>(optionDelta - 13);
                optionDelta = 13;
                isOptionDeltaExtended = true;
            }

            // Length > 12 : special values
            if (optionLength > 268) {
                optionLengthExtended = static_cast<quint8>(optionLength - 269);
                optionLength = 14;
                isOptionLengthExtended = true;
            } else if (optionLength > 12) {
                optionLengthExtended = static_cast<quint8>(optionLength - 13);
                optionLength = 13;
                isOptionLengthExtended = true;
            }

            optionPdu = static_cast<quint8>((static_cast<quint8>(optionDelta) << 4)         // Option Delta
                                            | (static_cast<quint8>(optionLength) & 0x0F));  // Option Length
            pdu.append(static_cast<char>(optionPdu));
            if (isOptionDeltaExtended)
                pdu.append(static_cast<char>(optionDeltaExtended));     // Option Delta Extended
            if (isOptionLengthExtended)
                pdu.append(static_cast<char>(optionLengthExtended));    // Option Length Extended
            pdu.append(option.value());                                 // Option Value

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!d->message->payload().isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d->message->payload());
    }

    return pdu;
}

void QCoapInternalRequest::setRequestToAskBlock(uint blockNumber, uint blockSize)
{
    // Set the Block2Option option to get the new block
    // size = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize)-4);
    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>(block2Data >> 8 & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d_func()->message->removeOptionByName(QCoapOption::Block2Option);
    d_func()->message->removeOptionByName(QCoapOption::Block1Option);
    addOption(QCoapOption::Block2Option, block2Value);

    d_func()->message->setMessageId(d_ptr->message->messageId()+1);
}

void QCoapInternalRequest::setRequestToSendBlock(uint blockNumber, uint blockSize)
{
    QCoapInternalRequestPrivate* d = d_func();

    d->message->setPayload(d->fullPayload.mid(blockNumber*blockSize, blockSize));

    // Set the Block2Option option to get the new block
    // size = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize)-4);
    if ((blockNumber * blockSize) + blockSize < d->fullPayload.length())
        block2Data = block2Data | 8; // Put the "more flag" to 1

    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>(block2Data >> 8 & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    d->message->removeOptionByName(QCoapOption::Block1Option);
    //removeOptionByName(QCoapOption::Block2Option);
    addOption(QCoapOption::Block1Option, block2Value);

    d->message->setMessageId(d_ptr->message->messageId()+1);
}

quint16 QCoapInternalRequest::generateMessageId()
{
    quint16 id = static_cast<quint16>(qrand() % 65536);
    d_func()->message->setMessageId(id);
    return id;
}

QByteArray QCoapInternalRequest::generateToken()
{
    QByteArray token("");
    quint8 length = (qrand() % 7) + 1;
    token.resize(length);

    quint8 *tokenData = reinterpret_cast<quint8 *>(token.data());
    for (int i = 0; i < token.size(); ++i)
        tokenData[i] = static_cast<quint8>(qrand() % 256);

    d_func()->message->setToken(token);
    return token;
}

void QCoapInternalRequest::addOption(const QCoapOption& option)
{
    QCoapInternalRequestPrivate* d = d_func();
    // If it is a BLOCK option, we need to know the block number
    if (option.name() == QCoapOption::Block1Option) {
        quint32 blockNumber = 0;
        quint8 *optionData = reinterpret_cast<quint8 *>(option.value().data());
        for (int i = 0; i < option.length() - 1; ++i)
            blockNumber = (blockNumber << 8) | optionData[i];
        blockNumber = (blockNumber << 4) | ((optionData[option.length()-1]) >> 4);
        d->currentBlockNumber = blockNumber;
        d->hasNextBlock = ((optionData[option.length()-1] & 0x8) == 0x8);
        d->blockSize = qPow(2, (optionData[option.length()-1] & 0x7) + 4);
    }

    d->message->addOption(option);
}

void QCoapInternalRequest::retransmit()
{
    QCoapInternalRequestPrivate* d = d_func();
    if (d->timeout <= 0)
        return;

    d->retransmissionCounter++;
    d->timeout *= 2;
    d->timer->start(static_cast<int>(d->timeout));
}

bool QCoapInternalRequest::isValid() const
{
    return d_func()->isValid;
}

QCoapConnection* QCoapInternalRequest::connection() const
{
    return d_func()->connection;
}

bool QCoapInternalRequest::cancelObserve() const
{
    return d_func()->cancelObserve;
}

QTimer* QCoapInternalRequest::timer() const
{
    return d_func()->timer;
}

uint QCoapInternalRequest::retransmissionCounter() const
{
    return d_func()->retransmissionCounter;
}

void QCoapInternalRequest::setOperation(QCoapOperation operation)
{
    QCoapInternalRequestPrivate* d = d_func();
    if (d->operation == operation)
        return;

    d->operation = operation;
}

void QCoapInternalRequest::setConnection(QCoapConnection* connection)
{
    QCoapInternalRequestPrivate* d = d_func();
    if (d->connection == connection)
        return;

    d->connection = connection;
}

void QCoapInternalRequest::setCancelObserve(bool cancelObserve)
{
    QCoapInternalRequestPrivate* d = d_func();
    if (d->cancelObserve == cancelObserve)
        return;

    d->cancelObserve = cancelObserve;
}

void QCoapInternalRequest::setTimeout(uint timeout)
{
    QCoapInternalRequestPrivate* d = d_func();
    d->timeout = timeout;
}

QCoapInternalRequestPrivate* QCoapInternalRequest::d_func() const
{
    return static_cast<QCoapInternalRequestPrivate*>(d_ptr);
}

bool QCoapInternalRequest::operator<(const QCoapInternalRequest& other) const
{
    return (d_ptr->message->token() < other.d_ptr->message->token());
}

/*void QCoapInternalRequestPrivate::_q_timeout()
{
    Q_Q(QCoapInternalRequest)
    emit q->timeout(this);
}*/
