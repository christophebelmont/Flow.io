#pragma once
/**
 * @file IIO.h
 * @brief I/O helper service interfaces.
 */
#include <stdint.h>

/** @brief Service interface for PCF8574 LED mask control exposed by IOModule. */
struct IOLedMaskService {
    bool (*setMask)(void* ctx, uint8_t mask);
    bool (*turnOn)(void* ctx, uint8_t bit);
    bool (*turnOff)(void* ctx, uint8_t bit);
    bool (*getMask)(void* ctx, uint8_t* mask);
    void* ctx;
};
