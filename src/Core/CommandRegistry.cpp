/**
 * @file CommandRegistry.cpp
 * @brief Implementation file.
 */
#include "CommandRegistry.h"
#include <cstring>
#include <cstdio>
#define LOG_TAG_CORE "CmdRegst"

bool CommandRegistry::registerHandler(const char* cmd, CommandHandler fn, void* userCtx) {
    if (!cmd || !fn) return false;
    if (count >= MAX_COMMANDS) return false;

    for (uint8_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].cmd, cmd) == 0) return false;
    }

    entries[count++] = {cmd, fn, userCtx};
    return true;
}

bool CommandRegistry::execute(const char* cmd, const char* json, const char* args, char* reply, size_t replyLen) {
    if (!cmd) return false;
    for (uint8_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].cmd, cmd) == 0) {
            CommandRequest req{cmd, json, args};
            return entries[i].fn(entries[i].userCtx, req, reply, replyLen);
        }
    }
    if (reply && replyLen) {
        snprintf(reply, replyLen, "{\"ok\":false,\"err\":\"unknown_cmd\"}");
    }
    return false;
}
