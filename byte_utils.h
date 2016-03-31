//
// Created by Austin Nodland on 12/4/15.
//

#ifndef DEVICE_LOGGER_C_UTILS_H
#define DEVICE_LOGGER_C_UTILS_H

#include <stdint.h>

void expandValue(uint32_t value, uint8_t *buffer);

uint32_t appendBytes(const uint8_t *bytes);

#endif //DEVICE_LOGGER_C_UTILS_H
