#pragma once
/**
 * @file IPreferences.h
 * @brief Preferences (NVS) service interface.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <Preferences.h>

/** @brief Service wrapper for Preferences instance. */
struct PreferencesService {
    Preferences* prefs;
};
