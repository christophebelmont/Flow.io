# Logging

## Components

Flow.IO logging is structured as:

- `LogHub`: central queue + formatting
- `LogSinkRegistry`: sink list and fan-out
- Sink modules (e.g. Serial sink)

## Logging macros

Modules typically include `Core/ModuleLog.h` and use:

```cpp
LOGI("hello %d", value);
LOGW("warning: %s", msg);
LOGE("error: %d", err);
```

## Log pipeline

1. Producer pushes `LogEntry` into LogHub queue.
2. LogDispatcher pulls entries and forwards them to sinks.
3. Sinks write to their backend (Serial, MQTT, file, ...).

## Threading

Logging is asynchronous: producers do not block on slow sinks.
