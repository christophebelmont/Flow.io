
# TimeModule – API Reference

## Services

### Exposed
- TimeService, TimeSchedulerService

### Consumed
- EventBusService, CommandService, DataStoreService

---

## Config Keys (NvsKeys)

- Time::Server1
- Time::Server2
- Time::Tz
- Time::ScheduleBlob

---

## DataKeys

- TimeSynced
- TimeEpoch
- SchedulerActiveMask

---

## EventBus

### Subscribed
- WifiNetReady
- ConfigChanged

### Emitted
- SchedulerEventTriggered

---

## SystemLimits

- TIME_SCHED_MAX_SLOTS
- Limits::JsonCmdTimeBuf

---

## Code Example

```cpp
// Example usage snippet
```
