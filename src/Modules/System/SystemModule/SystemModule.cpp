/**
 * @file SystemModule.cpp
 * @brief Implementation file.
 */
#include "SystemModule.h"
#include <esp_system.h>
#define LOG_TAG "SysModul"
#include "Core/ModuleLog.h"


bool SystemModule::cmdPing(void*, const CommandRequest&, char* reply, size_t replyLen) {
    snprintf(reply, replyLen, "{\"ok\":true,\"pong\":true}");
    return true;
}

bool SystemModule::cmdReboot(void*, const CommandRequest&, char* reply, size_t replyLen) {
    snprintf(reply, replyLen, "{\"ok\":true,\"msg\":\"rebooting\"}");
    delay(200); ///< laisser le temps à MQTT de publier l'ACK
    esp_restart();
    return true;
}

bool SystemModule::cmdFactoryReset(void* userCtx, const CommandRequest&, char* reply, size_t replyLen) {
    SystemModule* self = (SystemModule*)userCtx;

    snprintf(reply, replyLen, "{\"ok\":true,\"msg\":\"nvs_cleared\"}");

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
