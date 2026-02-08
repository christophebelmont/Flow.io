/**
 * @file IOScheduler.cpp
 * @brief Implementation file.
 */

#include "IOScheduler.h"

bool IOScheduler::add(const IOScheduledJob& job)
{
    if (!job.id || !job.fn) return false;
    if (count_ >= IO_SCHED_MAX_JOBS) return false;
    jobs_[count_++] = job;
    return true;
}

void IOScheduler::tick(uint32_t nowMs)
{
    for (uint8_t i = 0; i < count_; ++i) {
        IOScheduledJob& j = jobs_[i];
        if (!j.fn || j.periodMs == 0) continue;

        if ((uint32_t)(nowMs - j.lastRunMs) < j.periodMs) continue;

        j.fn(j.ctx, nowMs);
        j.lastRunMs = nowMs;
    }
}
