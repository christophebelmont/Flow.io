/**
 * @file LogHub.cpp
 * @brief Implementation file.
 */
#include "Core/LogHub.h"
#define LOG_TAG_CORE "LogHubMg"

void LogHub::init(int queueLen) {
    q = xQueueCreate(queueLen, sizeof(LogEntry));
}

bool LogHub::enqueue(const LogEntry& e) {
    if (!q) return false;
    return xQueueSend(q, &e, 0) == pdTRUE;  ///< 0 => non bloquant
}

bool LogHub::dequeue(LogEntry& out, TickType_t waitTicks) {
    if (!q) return false;
    return xQueueReceive(q, &out, waitTicks) == pdTRUE;
}
