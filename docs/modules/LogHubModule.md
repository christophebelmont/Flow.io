# LogHubModule

## Summary

**Module ID:** `loghub`  
**Type:** Active (FreeRTOS task)  
**Exposes services:** `loghub (LogHubService)`, `logsinks (LogSinkRegistryService)`  

## Purpose

LogHubModule hosts the central log queue and sink registry.

Example log macro:

```cpp
LOGI("mqtt connected");
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
