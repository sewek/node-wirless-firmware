#pragma once

#include <stdint.h>
#include "esp_gatt_defs.h"
#include "ble_interface.h"

#include "sdkconfig.h"

#ifdef CONFIG_BT_DEVICE_INFO_SERVICE_ENABLE
#include "services/device_info_service.h"
#endif

#ifdef CONFIG_BT_BATTERY_SERVICE_ENABLE
#include "services/battery_service.h"
#endif

const ble_gatt_server_service_t *ble_gatt_server_services[] = {

#ifdef CONFIG_BT_DEVICE_INFO_SERVICE_ENABLE
    &device_info_service,
#endif

#ifdef CONFIG_BT_BATTERY_SERVICE_ENABLE
    &battery_service,
#endif

};

const uint8_t ble_gatt_server_services_len = sizeof(ble_gatt_server_services) / sizeof(ble_gatt_server_services[0]);