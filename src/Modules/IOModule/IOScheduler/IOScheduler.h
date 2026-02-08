#pragma once
/**
 * @file IOScheduler.h
 * @brief Fixed-size cooperative scheduler.
 */

#include <stdint.h>

constexpr uint8_t IO_SCHED_MAX_JOBS = 16;

typedef bool (*IOScheduledFn)(void* ctx, uint32_t nowMs);

struct IOScheduledJob {
    const char* id = nullptr;
    uint32_t periodMs = 1000;
    uint32_t lastRunMs = 0;
    IOScheduledFn fn = nullptr;
    void* ctx = nullptr;
};

class IOScheduler {
public:
    bool add(const IOScheduledJob& job);
    void tick(uint32_t nowMs);

private:
    IOScheduledJob jobs_[IO_SCHED_MAX_JOBS]{};
    uint8_t count_ = 0;
};
