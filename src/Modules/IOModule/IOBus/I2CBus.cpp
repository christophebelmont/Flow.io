/**
 * @file I2CBus.cpp
 * @brief Implementation file.
 */

#include "I2CBus.h"

void I2CBus::begin(int sda, int scl, uint32_t frequencyHz)
{
    Wire.begin(sda, scl, frequencyHz);
    if (!mutex_) mutex_ = xSemaphoreCreateMutex();
}

bool I2CBus::lock(uint32_t timeoutMs)
{
    if (!mutex_) return false;
    return xSemaphoreTake(mutex_, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void I2CBus::unlock()
{
    if (!mutex_) return;
    xSemaphoreGive(mutex_);
}

bool I2CBus::probe(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool I2CBus::writeReg(uint8_t addr, uint8_t reg, const uint8_t* data, uint16_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    for (uint16_t i = 0; i < len; ++i) Wire.write(data[i]);
    return Wire.endTransmission() == 0;
}

bool I2CBus::readReg(uint8_t addr, uint8_t reg, uint8_t* data, uint16_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;

    uint16_t read = Wire.requestFrom(addr, (uint8_t)len);
    if (read != len) return false;

    for (uint16_t i = 0; i < len; ++i) data[i] = Wire.read();
    return true;
}

bool I2CBus::writeBytes(uint8_t addr, const uint8_t* data, uint16_t len)
{
    Wire.beginTransmission(addr);
    for (uint16_t i = 0; i < len; ++i) Wire.write(data[i]);
    return Wire.endTransmission() == 0;
}

bool I2CBus::readBytes(uint8_t addr, uint8_t* data, uint16_t len)
{
    uint16_t read = Wire.requestFrom(addr, (uint8_t)len);
    if (read != len) return false;

    for (uint16_t i = 0; i < len; ++i) data[i] = Wire.read();
    return true;
}
