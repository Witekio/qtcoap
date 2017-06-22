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
    INVALIDCODE = 0x00,
    CREATED = 0x41, // 2.01
    DELETED = 0x42, // 2.02
    VALID   = 0x43, // 2.03
    CHANGED = 0x44, // 2.04
    CONTENT = 0x45  // 2.05
    // TODO : add other status code
};

#endif // QCOAPGLOBAL_H
