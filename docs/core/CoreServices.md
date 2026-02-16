
# Core Services – API Reference

## ServiceRegistry

```cpp
services.add("serviceId", &serviceStruct);
auto svc = services.get<ServiceType>("serviceId");
```

## DataStoreService

```cpp
RuntimeData& dataMutable();
void notifyChanged(DataKey key, DirtyFlags flags);
```

## ConfigStoreService

```cpp
bool applyJson(const char* json, char* errBuf, size_t errLen);
void registerVar(ConfigVariableBase* var);
```

## EventBusService

```cpp
void publish(EventType type, const void* payload, size_t size);
void subscribe(EventType type, Callback cb, void* ctx);
```

## LogHubService

```cpp
LOGI("message");
LOGW("warning");
LOGE("error");
```
