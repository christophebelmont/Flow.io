# EventBus

## Purpose

EventBus is an internal pub/sub system for cross-module notifications with bounded memory.

## Event model

Events are identified by `EventType` and carry an optional payload (trivially copyable).

Examples:
- `WifiNetReady`
- `DataChanged`
- `ConfigChanged`
- `SchedulerEventTriggered`

## Subscription API

Modules subscribe using a callback + user context:

```cpp
eventBus->subscribe(EventType::ConfigChanged, &MyModule::onEventStatic, this);
```

## Dispatch

EventBus maintains a bounded FreeRTOS queue (`Limits::EventQueueLen`) and dispatches in its module loop.

## Payload rules

Payload structs live in `src/Core/EventBus/EventPayloads.h` and must remain small and trivially copyable.
