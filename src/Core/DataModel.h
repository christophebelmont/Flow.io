#pragma once
/**
 * @file DataModel.h
 * @brief Runtime data model types for the DataStore.
 */
#include <stdbool.h>

// Core fields are defined here (if any), and module fields are generated at build time.
#define RUNTIME_DATA_CORE_FIELDS

/** @brief Root runtime data model. */
#include "Core/Generated/ModuleDataModel_Generated.h"
