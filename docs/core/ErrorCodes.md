# Error Codes

Flow.IO uses a centralized error code enum and JSON writer helpers.

## ErrorCode

Defined in `include/Core/ErrorCodes.h`.

Key elements:
- `ErrorCode` enum
- `errorCodeStr(code)` → stable string
- `errorCodeRetryable(code)` → retry hint
- `writeErrorJson(...)` → standard JSON format

## Standard error JSON

Example payload:

```json
{
  "ok": false,
  "err": {
    "code": "BadCmdJson",
    "where": "cmd",
    "retryable": false
  }
}
```

Modules reuse this payload format for MQTT `ack`, `cfg/ack`, and command replies.
