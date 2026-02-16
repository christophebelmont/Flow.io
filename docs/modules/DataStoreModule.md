# DataStoreModule

## Summary

**Module ID:** `datastore`  
**Type:** Passive (no task)  
**Exposes services:** `datastore (DataStoreService)`  

## Purpose

DataStoreModule owns the root RuntimeData instance and exposes it through DataStoreService.

Example notify:

```cpp
ds.notifyChanged(DataKeys::WifiRssi, DIRTY_NETWORK);
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
