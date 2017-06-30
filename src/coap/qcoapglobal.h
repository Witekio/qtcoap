#ifndef QCOAPGLOBAL_H
#define QCOAPGLOBAL_H

enum QCoapOperation {
    EMPTY,
    GET,
    POST,
    PUT,
    DELETE,
    OTHER
};

enum QCoapStatusCode {
    InvalidCode = 0x00,
    CreatedCode = 0x41, // 2.01
    DeletedCode = 0x42, // 2.02
    ValidCode   = 0x43, // 2.03
    ChangedCode = 0x44, // 2.04
    ContentCode = 0x45,  // 2.05
    BadRequestCode = 0x80 // 4.00
    // TODO : add other status code
};

#endif // QCOAPGLOBAL_H
