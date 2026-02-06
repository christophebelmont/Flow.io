#pragma once
/**
 * @file CommandRegistry.h
 * @brief Command registration and execution.
 */
#include <stdint.h>
#include <stddef.h>

/** @brief Maximum number of registered commands. */
constexpr uint8_t MAX_COMMANDS = 24;

/** @brief Command invocation context. */
struct CommandRequest {
    const char* cmd;
    const char* json;
    const char* args;
};

/** @brief Handler signature for commands. */
using CommandHandler = bool (*)(void* userCtx,
                                const CommandRequest& req,
                                char* reply,
                                size_t replyLen);

/** @brief Registered command entry. */
struct CommandEntry {
    const char* cmd;
    CommandHandler fn;
    void* userCtx;
};

/**
 * @brief Registry of command handlers.
 */
class CommandRegistry {
public:
    /** @brief Register a handler for a command string. */
    bool registerHandler(const char* cmd, CommandHandler fn, void* userCtx);
    /** @brief Execute a command into a reply buffer. */
    bool execute(const char* cmd, const char* json, const char* args, char* reply, size_t replyLen);

private:
    CommandEntry entries[MAX_COMMANDS]{};
    uint8_t count = 0;
};
