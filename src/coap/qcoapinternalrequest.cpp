#include "qcoapinternalrequest_p.h"
#include "qcoaprequest.h"

QCoapInternalRequestPrivate::QCoapInternalRequestPrivate() :
    operation(EMPTY)
{
}

QCoapInternalRequest::QCoapInternalRequest() :
    d_ptr(new QCoapInternalRequestPrivate)
{
}

QCoapInternalRequest::QCoapInternalRequest(const QCoapRequest& request) :
    QCoapInternalRequest()
{
    d_ptr->version = request.version();
    d_ptr->type = request.type();
    d_ptr->messageId = request.messageId();
    d_ptr->token = request.token();
    d_ptr->tokenLength = request.tokenLength();
    for (int i = 0; i < request.optionsLength(); ++i)
        d_ptr->options.push_back(request.option(i));
    d_ptr->payload = request.payload();
    d_ptr->operation = request.operation();
}

QCoapInternalRequest QCoapInternalRequest::fromQCoapRequest(const QCoapRequest& request)
{
    QCoapInternalRequest internalRequest(request);
    return internalRequest;
}

QByteArray QCoapInternalRequest::toQByteArray() const
{
    QByteArray pdu;

    // Insert header
    quint32 coapHeader = (quint32(d_ptr->version) << 30)    // Coap version
            | (quint32(d_ptr->type) << 28)                  // Message type
            | (quint32(d_ptr->tokenLength) << 24)           // Token Length
            | (quint32(d_ptr->operation) << 16)             // Operation type
            | (quint32(d_ptr->messageId));                  // Message ID

    pdu.append(static_cast<quint8>(coapHeader >> 24));
    pdu.append(static_cast<quint8>((coapHeader >> 16) & 0xFF));
    pdu.append(static_cast<quint8>((coapHeader >> 8) & 0xFF));
    pdu.append(static_cast<quint8>(coapHeader & 0xFF));

    // Insert Token
    pdu.append(d_ptr->token);

    // Insert Options
    if (!d_ptr->options.isEmpty()) {
        // Sort options by ascending order
        qSort(d_ptr->options.begin(), d_ptr->options.end(),
              [](const QCoapOption& a, const QCoapOption& b) -> bool {
            return (a.name() < b.name());
        });

        quint8 lastOptionNumber = 0;
        for (QCoapOption option : d_ptr->options) {
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

            optionPdu = (static_cast<quint8>(optionDelta) << 4)          // Option Delta
                        | (static_cast<quint8>(optionLength) & 0x0F);    // Option Length
            pdu.append(optionPdu);
            if (isOptionDeltaExtended)
                pdu.append(optionDeltaExtended);    // Option Delta Extended
            if (isOptionLengthExtended)
                pdu.append(optionLengthExtended);   // Option Length Extended
            pdu.append(option.value());            // Option Value

            lastOptionNumber = option.name();
        }
    }

    // Insert Payload
    if (!payload().isEmpty()) {
        pdu.append(static_cast<char>(0xFF));
        pdu.append(d_ptr->payload);
    }

    return pdu;
}
