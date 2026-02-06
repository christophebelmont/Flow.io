#pragma once
/**
 * @file SystemModule.h
 * @brief System command module (ping/reboot/factory reset).
 */
#include "Core/ModulePassive.h"
#include "Core/Services/Services.h"

/**
 * @brief Passive module that registers system commands.
 */
class SystemModule : public ModulePassive {
public:
    /** @brief Module id. */
    const char* moduleId() const override { return "system"; }

    /** @brief Depends on log hub and command service. */
    uint8_t dependencyCount() const override { return 2; }
    const char* dependency(uint8_t i) const override {
        if (i == 0) return "loghub";
        if (i == 1) return "cmd";
        return nullptr;
    }

    /** @brief Register system commands. */
    void init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) override;

private:
    const CommandService* cmdSvc = nullptr;
    const LogHubService* logHub = nullptr;
    static bool cmdPing(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);
    static bool cmdReboot(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);
    static bool cmdFactoryReset(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);
};
