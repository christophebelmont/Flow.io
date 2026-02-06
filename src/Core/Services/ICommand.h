#pragma once
/**
 * @file ICommand.h
 * @brief Command service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>


/** @brief Forward declaration for command request struct. */
struct CommandRequest;
typedef bool (*CommandHandler)(void* userCtx, const CommandRequest& req, char* reply, size_t replyLen);

/** @brief Service interface for command registration and execution. */
struct CommandService {
    bool (*registerHandler)(void* ctx, const char* cmd, CommandHandler fn, void* userCtx);
    bool (*execute)(void* ctx, const char* cmd, const char* json, const char* args, char* reply, size_t replyLen);
    void* ctx;
};
