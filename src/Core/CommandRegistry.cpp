/**
 * @file CommandRegistry.cpp
 * @brief Implementation file.
 */
#include "CommandRegistry.h"
#include "Core/ErrorCodes.h"
#include "Core/SnprintfCheck.h"
#include <cstring>
#include <cstdio>
#define LOG_TAG_CORE "CmdRegst"
#undef snprintf
#define snprintf(OUT, LEN, FMT, ...) \
    FLOW_SNPRINTF_CHECKED(LOG_TAG_CORE, OUT, LEN, FMT, ##__VA_ARGS__)

static bool isJsonObjectReply_(const char* s, size_t len)
{
    if (!s || len == 0) return false;
    size_t i = 0;
    while (i < len) {
        const char c = s[i];
        if (c == '\0') return false;
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            return c == '{';
        }
        ++i;
    }
    return false;
}

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
    if (!cmd) {
        if (reply && replyLen) {
            if (!writeErrorJson(reply, replyLen, ErrorCode::UnknownCmd, "command")) {
                snprintf(reply, replyLen, "{\"ok\":false}");
            }
        }
        return false;
    }
    for (uint8_t i = 0; i < count; ++i) {
        if (strcmp(entries[i].cmd, cmd) == 0) {
            CommandRequest req{cmd, json, args};
            const bool ok = entries[i].fn(entries[i].userCtx, req, reply, replyLen);
            if (reply && replyLen) {
                if (!isJsonObjectReply_(reply, replyLen)) {
                    if (!writeErrorJson(reply, replyLen, ErrorCode::CmdHandlerFailed, "command.reply")) {
                        snprintf(reply, replyLen, "{\"ok\":false}");
                    }
                    return false;
                }
            }
            return ok;
        }
    }
    if (reply && replyLen) {
        if (!writeErrorJson(reply, replyLen, ErrorCode::UnknownCmd, "command")) {
            snprintf(reply, replyLen, "{\"ok\":false}");
        }
    }
    return false;
}
