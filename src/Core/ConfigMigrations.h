#pragma once
/**
 * @file ConfigMigrations.h
 * @brief Config migration steps for ConfigStore.
 */
#include <Preferences.h>
#include "Core/ConfigStore.h"
#include "Core/NvsKeys.h"

/** @brief Current configuration schema version. */
constexpr uint32_t CURRENT_CFG_VERSION = 3;

/** @brief Migration step from version 0 to 1. */
static bool mig_0_to_1(Preferences& prefs, bool clearOnFail)
{
    (void)prefs;
    (void)clearOnFail;
    // TODO migration logic
    return true; // true = OK, false = failed
}

/** @brief Migration step from version 1 to 2. */
static bool mig_1_to_2(Preferences& prefs, bool clearOnFail)
{
    (void)clearOnFail;
    if (prefs.isKey(NvsKeys::Hmi::RemoteUdpEnabledLegacy) &&
        !prefs.isKey(NvsKeys::Hmi::FlowConnectUdpEnabled)) {
        const bool enabled = prefs.getBool(NvsKeys::Hmi::RemoteUdpEnabledLegacy, false);
        (void)prefs.putBool(NvsKeys::Hmi::FlowConnectUdpEnabled, enabled);
    }
    if (prefs.isKey(NvsKeys::Hmi::RemoteUdpTokenLegacy) &&
        !prefs.isKey(NvsKeys::Hmi::FlowConnectUdpToken)) {
        char token[33]{};
        if (prefs.getString(NvsKeys::Hmi::RemoteUdpTokenLegacy, token, sizeof(token)) > 0U) {
            (void)prefs.putString(NvsKeys::Hmi::FlowConnectUdpToken, token);
        }
    }
    return true;
}

/** @brief Migration step from version 2 to 3. */
static bool mig_2_to_3(Preferences& prefs, bool clearOnFail)
{
    (void)clearOnFail;
#if defined(FLOW_BOARD_WAVESHARE_ESP32_S3)
    struct BindingMigration {
        const char* bindingKey;
        uint16_t bindingPort;
        const char* nameKey;
        const char* name;
    };
    static constexpr BindingMigration kBindings[] = {
        {NvsKeys::Io::IO_I8BP, 408U, NvsKeys::Io::IO_I8NM, "GPA0"},
        {NvsKeys::Io::IO_I9BP, 411U, NvsKeys::Io::IO_I9NM, "GPA3"},
        {NvsKeys::Io::IO_I10BP, 412U, NvsKeys::Io::IO_I10NM, "GPA4"},
        {NvsKeys::Io::IO_I11BP, 413U, NvsKeys::Io::IO_I11NM, "GPA5"},
        {NvsKeys::Io::IO_I12BP, 414U, NvsKeys::Io::IO_I12NM, "GPA6"},
        {NvsKeys::Io::IO_D8BP, 400U, NvsKeys::Io::IO_D8NM, "GPB0"},
        {NvsKeys::Io::IO_D9BP, 406U, NvsKeys::Io::IO_D9NM, "GPB6"},
    };
    for (const BindingMigration& binding : kBindings) {
        (void)prefs.putUShort(binding.bindingKey, binding.bindingPort);
        (void)prefs.putString(binding.nameKey, binding.name);
    }
    // The Waveshare profile now has only two MCP-backed complementary outputs.
    (void)prefs.putUShort(NvsKeys::Io::IO_D10BP, 0U);
    (void)prefs.putUShort(NvsKeys::Io::IO_D11BP, 0U);
#else
    (void)prefs;
#endif
    return true;
}

/** @brief Ordered list of migrations. */
static const MigrationStep steps[] = {
    {0, 1, mig_0_to_1},
    {1, 2, mig_1_to_2},
    {2, 3, mig_2_to_3}
};

/** @brief Number of migration steps. */
static constexpr size_t MIGRATION_COUNT = sizeof(steps) / sizeof(steps[0]);
