/**
 * @file EventBus.cpp
 * @brief Implementation file.
 */
#include "EventBus.h"
#include <Arduino.h>  // micros(), millis()
#include "Core/Log.h"

#define LOG_TAG_CORE "EventBus"

#if EVENTBUS_PROFILE
static uint32_t g_lastWarnMs = 0;
static bool canWarnNow() {
    uint32_t now = millis();
    if ((uint32_t)(now - g_lastWarnMs) < EVENTBUS_WARN_MIN_INTERVAL_MS) return false;
    g_lastWarnMs = now;
    return true;
}
#endif

EventBus::EventBus() {
    _queue = xQueueCreate(QUEUE_LENGTH, sizeof(QueuedEvent));
}

bool EventBus::subscribe(EventId id, EventCallback cb, void* user) {
    if (cb == nullptr) return false;
    if (_count >= MAX_SUBSCRIBERS) return false;

    _subs[_count].id = id;
    _subs[_count].cb = cb;
    _subs[_count].user = user;
    _count++;
    return true;
}

bool EventBus::post(EventId id, const void* payload, size_t len) {
    if (_queue == nullptr) return false;
    if (len > MAX_PAYLOAD_SIZE) return false;

    QueuedEvent qe;
    qe.id = id;
    qe.len = static_cast<uint8_t>(len);
    if (len > 0 && payload != nullptr) {
        memcpy(qe.data, payload, len);
    }

    /// non-blocking send (0 ticks) to keep real-time constraints
    BaseType_t ok = xQueueSend(_queue, &qe, 0);
    return ok == pdTRUE;
}

bool EventBus::postFromISR(EventId id, const void* payload, size_t len) {
    if (_queue == nullptr) return false;
    if (len > MAX_PAYLOAD_SIZE) return false;

    QueuedEvent qe;
    qe.id = id;
    qe.len = static_cast<uint8_t>(len);
    if (len > 0 && payload != nullptr) {
        memcpy(qe.data, payload, len);
    }

    BaseType_t higherWoken = pdFALSE;
    BaseType_t ok = xQueueSendFromISR(_queue, &qe, &higherWoken);
    if (higherWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
    return ok == pdTRUE;
}

void EventBus::dispatch(uint16_t maxEvents) {
    if (_queue == nullptr) return;

#if EVENTBUS_PROFILE
    const uint32_t tDispatch0 = micros();
    uint16_t dispatched = 0;
#endif

    for (uint16_t i = 0; i < maxEvents; i++) {
        QueuedEvent qe;
        BaseType_t ok = xQueueReceive(_queue, &qe, 0);
        if (ok != pdTRUE) {
            break;
        }

        dispatchOne(qe);

#if EVENTBUS_PROFILE
        dispatched++;
#endif
    }

#if EVENTBUS_PROFILE
    const uint32_t dt = (uint32_t)(micros() - tDispatch0);
    if (dispatched > 0 && dt > EVENTBUS_DISPATCH_WARN_US && canWarnNow()) {
        Log::warn(LOG_TAG_CORE,"dispatch slow: %u events dt=%lu us", (unsigned)dispatched, (unsigned long)dt);
    }
#endif
}

void EventBus::dispatchOne(const QueuedEvent& qe) {
    Event e;
    e.id = qe.id;
    e.payload = (qe.len > 0) ? qe.data : nullptr;
    e.len = qe.len;

    for (uint16_t i = 0; i < _count; i++) {
        if (_subs[i].id == qe.id && _subs[i].cb != nullptr) {

#if EVENTBUS_PROFILE
            const uint32_t t0 = micros();
#endif

            _subs[i].cb(e, _subs[i].user);

#if EVENTBUS_PROFILE
            const uint32_t dt = (uint32_t)(micros() - t0);
            if (dt > EVENTBUS_HANDLER_WARN_US && canWarnNow()) {
                Log::warn(LOG_TAG_CORE,"slow handler: event=%u cb=%p user=%p dt=%lu us",
                     (unsigned)qe.id,
                     (void*)_subs[i].cb,
                     _subs[i].user,
                     (unsigned long)dt);
            }
#endif
        }
    }
}
