#pragma once
/**
 * @file ITimeScheduler.h
 * @brief Scheduler service interface for time slots and reminders.
 */
#include <stdint.h>

/** @brief Maximum number of scheduler slots. */
constexpr uint8_t TIME_SCHED_MAX_SLOTS = 16;
constexpr uint8_t TIME_SCHED_LABEL_MAX = 24;

/** @brief Weekday bit constants (Mon..Sun). */
constexpr uint8_t TIME_WEEKDAY_MON = (1u << 0);
constexpr uint8_t TIME_WEEKDAY_TUE = (1u << 1);
constexpr uint8_t TIME_WEEKDAY_WED = (1u << 2);
constexpr uint8_t TIME_WEEKDAY_THU = (1u << 3);
constexpr uint8_t TIME_WEEKDAY_FRI = (1u << 4);
constexpr uint8_t TIME_WEEKDAY_SAT = (1u << 5);
constexpr uint8_t TIME_WEEKDAY_SUN = (1u << 6);

/** @brief Common recurrence masks. */
constexpr uint8_t TIME_WEEKDAY_NONE = 0x00;
constexpr uint8_t TIME_WEEKDAY_WORKDAYS =
    (TIME_WEEKDAY_MON | TIME_WEEKDAY_TUE | TIME_WEEKDAY_WED | TIME_WEEKDAY_THU | TIME_WEEKDAY_FRI);
constexpr uint8_t TIME_WEEKDAY_WEEKEND = (TIME_WEEKDAY_SAT | TIME_WEEKDAY_SUN);
constexpr uint8_t TIME_WEEKDAY_ALL = (TIME_WEEKDAY_WORKDAYS | TIME_WEEKDAY_WEEKEND);

/** @brief Reserved system slots (pre-configured by TimeModule). */
constexpr uint8_t TIME_SLOT_SYS_DAY_START = 0;
constexpr uint8_t TIME_SLOT_SYS_WEEK_START = 1;
constexpr uint8_t TIME_SLOT_SYS_MONTH_START = 2;
constexpr uint8_t TIME_SLOT_SYS_RESERVED_COUNT = 3;

/** @brief Reserved system event ids for scheduler notifications. */
constexpr uint16_t TIME_EVENT_SYS_DAY_START = 0xF001;
constexpr uint16_t TIME_EVENT_SYS_WEEK_START = 0xF002;
constexpr uint16_t TIME_EVENT_SYS_MONTH_START = 0xF003;

/** @brief Slot schedule mode. */
enum class TimeSchedulerMode : uint8_t {
    RecurringClock = 0, // recurring by local hour/minute + weekdayMask
    OneShotEpoch = 1    // single shot by epoch time
};

/**
 * @brief Unified scheduler slot definition.
 *
 * A slot with `hasEnd=false` behaves like an event/reminder.
 * A slot with `hasEnd=true` behaves like a time slot window (start/stop).
 */
struct TimeSchedulerSlot {
    uint8_t slot = 0;         // 0..TIME_SCHED_MAX_SLOTS-1
    uint16_t eventId = 0;     // consumer-level identifier
    bool enabled = true;
    bool hasEnd = false;
    bool replayStartOnBoot = true; // emit START replay when slot is already active at startup
    char label[TIME_SCHED_LABEL_MAX] = {0}; // optional debug label

    TimeSchedulerMode mode = TimeSchedulerMode::RecurringClock;

    // RecurringClock fields
    uint8_t weekdayMask = TIME_WEEKDAY_ALL; // bit0=Mon ... bit6=Sun
    uint8_t startHour = 0;
    uint8_t startMinute = 0;
    uint8_t endHour = 0;
    uint8_t endMinute = 0;

    // OneShotEpoch fields
    uint64_t startEpochSec = 0;
    uint64_t endEpochSec = 0;
};

/** @brief Service used by modules to register/update scheduler slots. */
struct TimeSchedulerService {
    bool (*setSlot)(void* ctx, const TimeSchedulerSlot* slotDef);
    bool (*getSlot)(void* ctx, uint8_t slot, TimeSchedulerSlot* outDef);
    bool (*clearSlot)(void* ctx, uint8_t slot);
    bool (*clearAll)(void* ctx);
    uint8_t (*usedCount)(void* ctx);
    uint16_t (*activeMask)(void* ctx);  // bit N set => slot N currently active
    bool (*isActive)(void* ctx, uint8_t slot);
    void* ctx;
};
