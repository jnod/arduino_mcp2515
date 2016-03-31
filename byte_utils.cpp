//
// Created by Austin Nodland on 12/4/15.
//

#include "byte_utils.h"

// Expands into little-endian byte order
void expandValue(uint32_t value, uint8_t *buffer) {
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (uint8_t) (value & 0xff);
        buffer[i] = byte;

        value >>= 8;
    }
}

// Appends as if the bytes are in little-endian byte order
uint32_t appendBytes(const uint8_t *bytes) {
    uint32_t output = 0;

    for (int i = 0; i < 4; i++) {
        output <<= 8;
        output |= bytes[3-i];
    }

    return output;
}