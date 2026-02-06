#pragma once
/**
 * @file ModulePassive.h
 * @brief Base class for passive (no-task) modules.
 */
#include "Core/Module.h"

/**
 * @brief Base class for modules that only register services or wiring.
 */
class ModulePassive : public Module {
public:
    /** @brief Passive modules do not create a task. */
    bool hasTask() const override { return false; }

    /** @brief No task name for passive modules. */
    const char* taskName() const override { return ""; }

    /** @brief Never called because no task is created. */
    void loop() override {}
};
