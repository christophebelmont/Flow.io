#pragma once
/**
 * @file EventBus.h
 * @brief Simple queued event bus with fixed-size payloads.
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "EventId.h"
#include "Core/SystemLimits.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifndef EVENTBUS_PROFILE
#define EVENTBUS_PROFILE 1
#endif

#ifndef EVENTBUS_HANDLER_WARN_US
#define EVENTBUS_HANDLER_WARN_US 5000  // 5ms
#endif

#ifndef EVENTBUS_DISPATCH_WARN_US
#define EVENTBUS_DISPATCH_WARN_US 20000 // 20ms for a batch
#endif

#ifndef EVENTBUS_WARN_MIN_INTERVAL_MS
#define EVENTBUS_WARN_MIN_INTERVAL_MS 2000
#endif

/** @brief Event delivered to subscribers during dispatch(). */
struct Event {
    EventId id;
    const void* payload;
    size_t len;
};

/** @brief Callback signature for event subscribers. */
using EventCallback = void(*)(const Event& e, void* user);

/**
 * @brief Thread-safe event queue with subscriber dispatch.
 */
class EventBus {
public:
    static constexpr uint16_t MAX_SUBSCRIBERS = 24;

    // Maximum payload size copied into internal queue.
    static constexpr uint8_t MAX_PAYLOAD_SIZE = 48;

    // Maximum number of queued events (FreeRTOS queue length).
    static constexpr uint8_t QUEUE_LENGTH = Limits::EventQueueLen;

    /** @brief Construct and initialize the event queue. */
    EventBus();
    ~EventBus() = default;

    /** @brief Subscribe to an event id (not thread-safe; call during init). */
    bool subscribe(EventId id, EventCallback cb, void* user);

    /**
     * @brief Post an event from any task; payload is copied into the queue.
     */
    bool post(EventId id, const void* payload = nullptr, size_t len = 0);

    /** @brief Post an event from ISR context. */
    bool postFromISR(EventId id, const void* payload = nullptr, size_t len = 0);

    /** @brief Dispatch queued events and call subscribers. */
    void dispatch(uint16_t maxEvents = 8);

private:
    struct Subscriber {
        EventId id;
        EventCallback cb;
        void* user;
    };

    struct QueuedEvent {
        EventId id;
        uint8_t len;
        uint8_t data[MAX_PAYLOAD_SIZE];
    };

    Subscriber _subs[MAX_SUBSCRIBERS];
    uint16_t _count = 0;

    QueueHandle_t _queue = nullptr;

    void dispatchOne(const QueuedEvent& qe);
};
