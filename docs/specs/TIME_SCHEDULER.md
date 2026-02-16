# Time Scheduler Model

TimeModule provides:
- NTP synchronization
- local time formatting (TZ + DST rules)
- a slot-based weekly scheduler
- scheduler edge events delivered via EventBus
- a scheduler service (`TimeSchedulerService`) used by other modules

## Persistent model

Slots are serialized into a bounded blob stored under `NvsKeys::Time::ScheduleBlob`.

## Edge events

TimeModule emits `SchedulerEventTriggered` events containing:
- slot index
- edge type: Trigger / Start / Stop
- epoch time
- active mask

Consumers can rely on deterministic evaluation and replayed edges at boot/resync.
