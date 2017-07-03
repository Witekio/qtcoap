#include "qcoapinternalrequest_p.h"
#include "qcoaprequest.h"

QCoapInternalRequestPrivate::QCoapInternalRequestPrivate() :
    operation(EMPTY),
    isValid(true),
    cancelObserve(false)
{
}

QCoapInternalRequest::QCoapInternalRequest() :
    QCoapInternalMessage(*new QCoapInternalRequestPrivate)
{
}

QCoapInternalRequest::QCoapInternalRequest(const QCoapRequest& request) :
    QCoapInternalRequest()
{
    QCoapInternalRequestPrivate* d = d_func();
    d->version = request.version();
    d->type = request.type();
    d->messageId = request.messageId();
    d->token = request.token();
    //d->tokenLength = request.tokenLength();
    for (int i = 0; i < request.optionsLength(); ++i)
        d->options.push_back(request.option(i));
    d->payload = request.payload();
    d->operation = request.operation();
}

QCoapInternalRequest QCoapInternalRequest::fromQCoapRequest(const QCoapRequest& request)
{
    QCoapInternalRequest internalRequest(request);
    return internalRequest;
}

QCoapInternalRequest QCoapInternalRequest::createAcknowledgment(quint16 messageId,
                                                                const QByteArray& token)
{
    QCoapInternalRequest internalRequest;

    internalRequest.setType(ACKNOWLEDGMENT);
    internalRequest.setOperation(EMPTY);
    internalRequest.setMessageId(messageId);
    internalRequest.setToken(token);
    internalRequest.setPayload(QByteArray());
    internalRequest.removeAllOptions();

    return internalRequest;
}

QCoapInternalRequest QCoapInternalRequest::createReset(quint16 messageId)
{
    QCoapInternalRequest internalRequest;

    internalRequest.setType(RESET);
    internalRequest.setOperation(EMPTY);
    internalRequest.setMessageId(messageId);
    internalRequest.setToken(QByteArray());
    internalRequest.setPayload(QByteArray());
    internalRequest.removeAllOptions();

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
    quint32 coapHeader = (quint32(d->version) << 30)    // Coap version
            | (quint32(d->type) << 28)                  // Message type
            | (quint32(d->token.length()) << 24)           // Token Length
            | (quint32(d->operation) << 16)             // Operation type
            | (quint32(d->messageId));                  // Message ID

    pdu.append(static_cast<char>(coapHeader >> 24));
    pdu.append(static_cast<char>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<char>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<char>(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d->token);

    // Insert Options
    if (!d->options.isEmpty()) {
        // Sort options by ascending order
        qSort(d->options.begin(), d->options.end(),
              [](const QCoapOption& a, const QCoapOption& b) -> bool {
            return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (QCoapOption option : d->options) {
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
    if (!d->payload.isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d->payload);
    }

    return pdu;
}

void QCoapInternalRequest::setRequestToAskBlock(uint blockNumber, uint blockSize)
{
    // Set the BLOCK2 option to get the new block
    // size = (2^(SZX + 4))
    quint32 block2Data = (blockNumber << 4) | static_cast<quint32>(log2(blockSize)-4);
    QByteArray block2Value = QByteArray();
    if (block2Data > 0xFFFF)
        block2Value.append(static_cast<char>(block2Data >> 16));
    if (block2Data > 0xFF)
        block2Value.append(static_cast<char>(block2Data >> 8 & 0xFF));
    block2Value.append(static_cast<char>(block2Data & 0xFF));

    removeOptionByName(QCoapOption::BLOCK2);
    addOption(QCoapOption::BLOCK2, block2Value);

    setMessageId(d_ptr->messageId+1);
}

quint16 QCoapInternalRequest::generateMessageId()
{
    quint16 id = static_cast<quint16>(qrand() % 65536);
    setMessageId(id);
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

    setToken(token);
    return token;
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

QCoapInternalRequestPrivate* QCoapInternalRequest::d_func() const
{
    return static_cast<QCoapInternalRequestPrivate*>(d_ptr);
}

bool QCoapInternalRequest::operator<(const QCoapInternalRequest& other) const
{
    return (d_ptr->token < other.token());
}
