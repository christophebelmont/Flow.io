# ConfigStoreModule

## Summary

**Module ID:** `config`  
**Type:** Passive (no task)  
**Exposes services:** `config (ConfigStoreService)`  

## Configuration

ConfigStore keys:

- `NvsKeys::StorageNamespace`
- `NvsKeys::ConfigVersion`

## Purpose

ConfigStoreModule exposes ConfigStore as a service and manages NVS-backed persistence.

Example apply:

```cpp
bool ok = cfg.applyJson(patchJson, errBuf, sizeof(errBuf));
```

## Quality Gate Score

See `docs/README.md` for the full quality-gate table.
