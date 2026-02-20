#pragma once
/**
 * @file ConfigStoreModule.h
 * @brief Module that exposes ConfigStore service.
 */
#include "Core/ModulePassive.h"
#include "Core/Services/Services.h"

/**
 * @brief Passive module wiring ConfigStore JSON services.
 */
class ConfigStoreModule : public ModulePassive {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "config"; }

    /** @brief Config module depends on log hub. */
    uint8_t dependencyCount() const override { return 1; }
    const char* dependency(uint8_t i) const override { return (i == 0) ? "loghub" : nullptr; }

    /** @brief Register config services. */
    void init(ConfigStore& cfg, ServiceRegistry& services) override;

private:
    ConfigStore* registry = nullptr;
    const LogHubService* logHub = nullptr;

    static bool svcApplyJson(void* ctx, const char* json);
    static void svcToJson(void* ctx, char* out, size_t outLen);
    static bool svcToJsonModule(void* ctx, const char* module, char* out, size_t outLen, bool* truncated);
    static uint8_t svcListModules(void* ctx, const char** out, uint8_t max);
    static bool svcErase(void* ctx);
};
