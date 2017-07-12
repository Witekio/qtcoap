#ifndef QCOAPGLOBAL_H
#define QCOAPGLOBAL_H

enum QCoapOperation {
    EmptyCoapOperation,
    GetCoapOperation,
    PostCoapOperation,
    PutCoapOperation,
    DeleteCoapOperation,
    OtherCoapOperation
};

enum QCoapStatusCode {
    InvalidCoapCode = 0x00,
    CreatedCoapCode = 0x41, // 2.01
    DeletedCoapCode = 0x42, // 2.02
    ValidCoapCode   = 0x43, // 2.03T
    ChangedCoapCode = 0x44, // 2.04
    ContentCoapCode = 0x45, // 2.05
    ContinueCoapCode = 0x5F, // 2.31
    BadRequestCoapCode = 0x80, // 4.00
    UnauthorizedCoapCode = 0x81, // 4.01
    BadOptionCoapCode = 0x82, // 4.02
    ForbiddenCoapCode = 0x83, // 4.03
    NotFoundCoapCode = 0x84, // 4.04
    MethodNotAllowedCoapCode = 0x85, // 4.05
    NotAcceptableCoapCode = 0x86, // 4.06
    RequestEntityIncompleteCoapCode = 0x88, // 4.08
    PreconditionFailedCoapCode = 0x8C,  // 4.12
    RequestEntityTooLargeCoapCode = 0x8D,   // 4.13
    UnsupportedContentFormatCoapCode = 0x8E, // 4.14
    InternalServerErrorCoapCode = 0xA0, // 5.00
    NotImplementedCoapCode = 0xA1,  // 5.01
    BadGatewayCoapCode = 0xA2,  // 5.02
    ServiceUnavailableCoapCode = 0xA3,  // 5.03
    GatewayTimeoutCoapCode = 0xA4,  // 5.04
    ProxyingNotSupportedCoapCode = 0xA5 // 5.05
};

#endif // QCOAPGLOBAL_H
