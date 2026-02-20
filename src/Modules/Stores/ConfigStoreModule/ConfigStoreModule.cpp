/**
 * @file ConfigStoreModule.cpp
 * @brief Implementation file.
 */
#include "ConfigStoreModule.h"
#define LOG_TAG "CfgModul"
#include "Core/ModuleLog.h"


bool ConfigStoreModule::svcApplyJson(void* ctx, const char* json) {
    return ((ConfigStore*)ctx)->applyJson(json);
}

void ConfigStoreModule::svcToJson(void* ctx, char* out, size_t outLen) {
    ((ConfigStore*)ctx)->toJson(out, outLen);
}

bool ConfigStoreModule::svcToJsonModule(void* ctx, const char* module, char* out, size_t outLen, bool* truncated) {
    return ((ConfigStore*)ctx)->toJsonModule(module, out, outLen, truncated);
}

uint8_t ConfigStoreModule::svcListModules(void* ctx, const char** out, uint8_t max) {
    return ((ConfigStore*)ctx)->listModules(out, max);
}

bool ConfigStoreModule::svcErase(void* ctx) {
    return ((ConfigStore*)ctx)->erasePersistent();
}

void ConfigStoreModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    registry = &cfg;

    /// récupérer service loghub (log async)
    logHub = services.get<LogHubService>("loghub");

    static ConfigStoreService svc{ svcApplyJson, svcToJson, svcToJsonModule, svcListModules, svcErase, nullptr };
    svc.ctx = registry;

    services.add("config", &svc);
    LOGI("ConfigStoreService registered");
}
