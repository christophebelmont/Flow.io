# TimeModule

## Summary

**Module ID:** `time`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `datastore`, `cmd`, `eventbus`  
**Exposes services:** `time (TimeService)`, `time.scheduler (TimeSchedulerService)`  
**Consumes services:** `wifi (indirect via network readiness events)`, `cmd`, `eventbus`, `datastore`, `loghub`  

## Purpose

TimeModule provides time synchronization and a slot-based scheduler.

It exposes:
- TimeService: epoch time, synced state, local time formatting
- TimeSchedulerService: a weekly scheduler with deterministic evaluation and edge events

## Configuration

ConfigStore variables (NVS keys):
- `NvsKeys::Time::Server1`
- `NvsKeys::Time::Server2`
- `NvsKeys::Time::Tz`
- `NvsKeys::Time::Enabled`
- `NvsKeys::Time::WeekStartMonday`
- `NvsKeys::Time::ScheduleBlob` (`time/scheduler` group)

## Scheduler model

Slots are stored in a bounded serialized blob and loaded into a fixed-size array:

```cpp
static constexpr size_t TIME_SCHED_BLOB_SIZE = 1536;
SchedulerSlotRuntime sched_[TIME_SCHED_MAX_SLOTS]{};
```

The module periodically evaluates current local time and updates active mask and edge transitions.

## EventBus integration

TimeModule emits scheduler edge events:

- `EventType::SchedulerEventTriggered`
- payload: `SchedulerEventTriggeredPayload` (slot, edge, epoch, active mask)

Example emission:

```cpp
SchedulerEventTriggeredPayload p{};
p.slot = slotIdx;
p.edge = (uint8_t)SchedulerEdge::Start;
p.epochSec = now;
eventBus->publish(EventType::SchedulerEventTriggered, &p, sizeof(p));
```

## Commands

TimeModule registers commands through CommandModule (CommandService), including a scheduler API:

- `time.resync`
- `time.scheduler.info`
- `time.scheduler.get`
- `time.scheduler.set`
- `time.scheduler.clear`
- `time.scheduler.clearAll`

Scheduler command arguments are parsed using `Limits::JsonCmdTimeBuf`.

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
