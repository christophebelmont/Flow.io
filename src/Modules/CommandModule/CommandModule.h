#pragma once
/**
 * @file CommandModule.h
 * @brief Module that exposes the command registry service.
 */
#include "Core/ModulePassive.h"
#include "Core/CommandRegistry.h"
#include "Core/Services/Services.h"

/**
 * @brief Passive module wiring command registration/execution service.
 */
class CommandModule : public ModulePassive {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "cmd"; }

    /** @brief Command module depends on log hub. */
    uint8_t dependencyCount() const override { return 1; }
    const char* dependency(uint8_t i) const override { return (i == 0) ? "loghub" : nullptr; }

    /** @brief Initialize registry and register service. */
    void init(ConfigStore&, ServiceRegistry& services) override;

private:
    CommandRegistry registry;
    const LogHubService* logHub = nullptr;

    static bool svcRegister(void* ctx, const char* cmd, CommandHandler fn, void* userCtx);
    static bool svcExecute(void* ctx, const char* cmd, const char* json, const char* args,
                           char* reply, size_t replyLen);
};
