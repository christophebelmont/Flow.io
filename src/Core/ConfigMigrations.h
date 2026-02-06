#pragma once
/**
 * @file ConfigMigrations.h
 * @brief Config migration steps for ConfigStore.
 */
#include <Preferences.h>
#include "Core/ConfigStore.h"

/** @brief Current configuration schema version. */
constexpr uint32_t CURRENT_CFG_VERSION = 1;

/** @brief Migration step from version 0 to 1. */
static bool mig_0_to_1(Preferences& prefs, bool clearOnFail)
{
    // TODO migration logic
    return true; // true = OK, false = failed
}

/** @brief Ordered list of migrations. */
static const MigrationStep steps[] = {
    {0, 1, mig_0_to_1}
};

/** @brief Number of migration steps. */
static constexpr size_t MIGRATION_COUNT = sizeof(steps) / sizeof(steps[0]);
