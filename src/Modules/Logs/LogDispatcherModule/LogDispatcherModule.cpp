/**
 * @file LogDispatcherModule.cpp
 * @brief Implementation file.
 */
#include "LogDispatcherModule.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct DispatcherCtx {
    LogHub* hub;
    const LogSinkRegistryService* sinks;
};

static DispatcherCtx g_ctx;

void LogDispatcherModule::init(ConfigStore& cfg, I2CManager& i2c, ServiceRegistry& services) {
    (void)cfg;
    (void)i2c;

    _services = &services;

    /// récupérer hub et sink registry
    auto hubSvc = services.get<LogHubService>("loghub");
    _sinkReg = services.get<LogSinkRegistryService>("logsinks");

    /// hub object dans ctx => on va le passer en brut via ctx->hub
    /// On va récupérer le LogHub* via hubSvc->ctx
    if (!hubSvc || !hubSvc->ctx || !_sinkReg) return;

    _hub = static_cast<LogHub*>(hubSvc->ctx);

    g_ctx.hub = _hub;
    g_ctx.sinks = _sinkReg;

    /// Crée la task log
    xTaskCreatePinnedToCore(
        LogDispatcherModule::taskFn,
        "LogDispatch",
        4096,                ///< stack
        &g_ctx,              ///< param
        1,                   ///< priorité basse
        nullptr,
        1                    ///< core 1 (optionnel)
    );
}

void LogDispatcherModule::taskFn(void* pv) {
    auto* ctx = static_cast<DispatcherCtx*>(pv);
    LogEntry e;

    while (true) {
        if (ctx->hub->dequeue(e, portMAX_DELAY)) {
            int n = ctx->sinks->count(ctx->sinks->ctx);

            for (int i = 0; i < n; ++i) {
                LogSinkService sink = ctx->sinks->get(ctx->sinks->ctx, i);
                if (sink.write) sink.write(sink.ctx, e);
            }
        }
    }
}
