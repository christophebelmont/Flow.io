/**
 * @file CommandModule.cpp
 * @brief Implementation file.
 */
#include "CommandModule.h"
#define LOG_TAG "CmdModul"
#include "Core/ModuleLog.h"


bool CommandModule::svcRegister(void* ctx, const char* cmd, CommandHandler fn, void* userCtx) {
    return ((CommandRegistry*)ctx)->registerHandler(cmd, fn, userCtx);
}

bool CommandModule::svcExecute(void* ctx, const char* cmd, const char* json, const char* args,
                               char* reply, size_t replyLen) {
    return ((CommandRegistry*)ctx)->execute(cmd, json, args, reply, replyLen);
}

void CommandModule::init(ConfigStore&, I2CManager&, ServiceRegistry& services) {
    static CommandService svc{ svcRegister, svcExecute, nullptr };
    svc.ctx = &registry;
    services.add("cmd", &svc);
    
    logHub = services.get<LogHubService>("loghub");
    LOGI("CommandService registered");
}
