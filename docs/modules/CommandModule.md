# CommandModule

## Summary

**Module ID:** `cmd`  
**Type:** Passive (no task)  
**Exposes services:** `cmd (CommandService)`  

## Purpose

CommandModule provides a command registry (`module.command`) and dispatch interface.

Example registration:

```cpp
cmd->registerHandler("time.resync", &TimeModule::cmdResync, this);
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
