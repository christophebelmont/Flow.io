/**
 * @file IDataStore.h
 * @brief Data store service interface.
 */
#include "Core/DataStore/DataStore.h"

/** @brief Service wrapper for access to the DataStore instance. */
struct DataStoreService {
    DataStore* store;
};
