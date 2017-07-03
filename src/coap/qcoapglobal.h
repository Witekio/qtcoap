#ifndef QCOAPGLOBAL_H
#define QCOAPGLOBAL_H

enum QCoapOperation {
    EmptyOperation,
    GetOperation,
    PostOperation,
    PutOperation,
    DeleteOperation,
    OtherOperation
};

enum QCoapStatusCode {
    InvalidCode = 0x00,
    CreatedCode = 0x41, // 2.01
    DeletedCode = 0x42, // 2.02
    ValidCode   = 0x43, // 2.03
    ChangedCode = 0x44, // 2.04
    ContentCode = 0x45,  // 2.05
    BadRequestCode = 0x80, // 4.00
    UnauthorizedCode = 0x81, // 4.01
    BadOptionCode = 0x82,   // 4.02
    ForbiddenCode = 0x83,   // 4.03
    NotFoundCode = 0x84,    // 4.04
    MethodNotAllowedCode = 0x85, // 4.05
    NotAcceptableCode = 0x86, // 4.06
    PreconditionFailedCode = 0x8C,  // 4.12
    RequestEntityTooLargeCode = 0x8D,   // 4.13
    UnsupportedContentFormatCode = 0x8E, // 4.14
    InternalServerErrorCode = 0xA0, // 5.00
    NotImplementedCode = 0xA1,  // 5.01
    BadGatewayCode = 0xA2,  // 5.02
    ServiceUnavailableCode = 0xA3,  // 5.03
    GatewayTimeoutCode = 0xA4,  // 5.04
    ProxyingNotSupportedCode = 0xA5 // 5.05
};

#endif // QCOAPGLOBAL_H
