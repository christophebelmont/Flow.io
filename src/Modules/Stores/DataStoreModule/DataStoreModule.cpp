/**
 * @file DataStoreModule.cpp
 * @brief Implementation file.
 */
#include "DataStoreModule.h"

void DataStoreModule::init(ConfigStore&, ServiceRegistry& services)
{
    auto* eb = services.get<EventBusService>("eventbus");
    if (eb && eb->bus) {
        _store.setEventBus(eb->bus);
    }

    services.add("datastore", &_svc);
}
