# SystemModule

## Summary

**Module ID:** `system`  
**Type:** Active (FreeRTOS task)  
**Declared dependencies:** `loghub`, `datastore`  
**Exposes services:** `system (SystemService)`  
**Consumes services:** `loghub`, `datastore`  

## Purpose

SystemModule publishes basic system runtime information (uptime, reset reason, heap stats) through DataStore snapshots.

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
