/**
 * @file SystemModule.cpp
 * @brief Implementation file.
 */
#include "SystemModule.h"
#include "Core/ErrorCodes.h"
#include <esp_system.h>
#define LOG_TAG "SysModul"
#include "Core/ModuleLog.h"

static bool writeOkReply_(char* reply, size_t replyLen, const char* json, const char* where)
{
    if (!reply || replyLen == 0 || !json) return false;
    const int wrote = snprintf(reply, replyLen, "%s", json);
    if (wrote > 0 && (size_t)wrote < replyLen) return true;
    if (!writeErrorJson(reply, replyLen, ErrorCode::Failed, where)) {
        snprintf(reply, replyLen, "{\"ok\":false}");
    }
    return false;
}

bool SystemModule::cmdPing(void*, const CommandRequest&, char* reply, size_t replyLen) {
    return writeOkReply_(reply, replyLen, "{\"ok\":true,\"pong\":true}", "system.ping");
}

bool SystemModule::cmdReboot(void*, const CommandRequest&, char* reply, size_t replyLen) {
    if (!writeOkReply_(reply, replyLen, "{\"ok\":true,\"msg\":\"rebooting\"}", "system.reboot")) {
        return false;
    }
    delay(200); ///< laisser le temps à MQTT de publier l'ACK
    esp_restart();
    return true;
}

bool SystemModule::cmdFactoryReset(void* userCtx, const CommandRequest&, char* reply, size_t replyLen) {
    (void)userCtx;

    if (!writeOkReply_(reply, replyLen, "{\"ok\":true,\"msg\":\"nvs_cleared\"}", "system.factory_reset")) {
        return false;
    }

    /* Reinit configuration is better in ConfigModule
    if (self->prefs) {
        self->prefs->clear();
    }*/

    delay(500); ///< laisser le temps à MQTT de publier l'ACK
    esp_restart();
    return true;
}

void SystemModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    (void)cfg;

    logHub = services.get<LogHubService>("loghub");
    cmdSvc = services.get<CommandService>("cmd");

    cmdSvc->registerHandler(cmdSvc->ctx, "system.ping", cmdPing, this);
    cmdSvc->registerHandler(cmdSvc->ctx, "system.reboot", cmdReboot, this);
    cmdSvc->registerHandler(cmdSvc->ctx, "system.factory_reset", cmdFactoryReset, this);

    LOGI("Commands registered: system.ping system.reboot system.factory_reset");
}
