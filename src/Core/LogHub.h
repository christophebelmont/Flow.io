#pragma once
/**
 * @file LogHub.h
 * @brief Central log queue for asynchronous logging.
 */
#include "Core/Services/ILogger.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/**
 * @brief Queue-based log hub for producers and consumers.
 */
class LogHub {
public:
    /** @brief Initialize the log queue with a given length. */
    void init(int queueLen = 32);

    /** @brief Enqueue a log entry (non-blocking). */
    bool enqueue(const LogEntry& e);
    /** @brief Dequeue a log entry (blocking up to waitTicks). */
    bool dequeue(LogEntry& out, TickType_t waitTicks);

private:
    QueueHandle_t q = nullptr;
};
