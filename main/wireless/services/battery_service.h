#ifndef __BATTERY_SERVICE_H__
#define __BATTERY_SERVICE_H__

#include "ble_interface.h"

#include <stdint.h>

#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"

enum
{
    BATTERY_SERVICE_SVC,

    BATTERY_SERVICE_CHAR,
    BATTERY_SERVICE_VALUE,
    BATTERY_SERVICE_DESC,
    BATTERY_SERVICE_CONF,

    BATTERY_SERVICE_NUM_ATTR,
};

extern uint16_t battery_service_handle_table[];
extern const uint16_t uuid_BATTERY_SERVICE_SVC;
extern const esp_gatts_attr_db_t battery_service_gatt_db[BATTERY_SERVICE_NUM_ATTR];
extern const ble_gatt_server_service_t battery_service;

#endif // __BATTERY_SERVICE_H__