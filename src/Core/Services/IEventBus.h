#pragma once
/**
 * @file IEventBus.h
 * @brief Event bus service interface.
 */

/** @brief Service wrapper for access to the EventBus instance. */
struct EventBusService {
    class EventBus* bus;
};
