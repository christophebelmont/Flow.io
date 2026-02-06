#pragma once
/**
 * @file II2C.h
 * @brief I2C service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

/** @brief Service interface for thread-safe I2C operations. */
struct I2CService {
    bool (*probe)(void* ctx, uint8_t addr);

    bool (*readReg)(void* ctx, uint8_t addr, uint8_t reg,
                    uint8_t* data, uint16_t len, uint32_t timeoutMs);

    bool (*writeReg)(void* ctx, uint8_t addr, uint8_t reg,
                     const uint8_t* data, uint16_t len, uint32_t timeoutMs);

    void* ctx;
};
