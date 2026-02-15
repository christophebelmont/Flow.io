/**
 * @file LogHubLModule.cpp
 * @brief Implementation file.
 */
#include "LogHubModule.h"
#include "Core/Log.h"
#include "Core/SystemLimits.h"

void LogHubModule::init(ConfigStore& cfg, ServiceRegistry& services) {
    (void)cfg;

    hub.init(Limits::LogQueueLen);

    /// expose loghub service
    hubSvc.enqueue = [](void* ctx, const LogEntry& e) -> bool {
        return static_cast<LogHub*>(ctx)->enqueue(e);
    };
    hubSvc.ctx = &hub;

    /// expose sink registry service
    sinksSvc.add = [](void* ctx, LogSinkService sink) -> bool {
        return static_cast<LogSinkRegistry*>(ctx)->add(sink);
    };
    sinksSvc.count = [](void* ctx) -> int {
        return static_cast<LogSinkRegistry*>(ctx)->count();
    };
    sinksSvc.get = [](void* ctx, int idx) -> LogSinkService {
        return static_cast<LogSinkRegistry*>(ctx)->get(idx);
    };
    sinksSvc.ctx = &sinks;

    services.add("loghub", &hubSvc);
    services.add("logsinks", &sinksSvc);

    Log::setHub(&hubSvc);

}
