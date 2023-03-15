#pragma once

#include "ble_interface.h"

#include <stdint.h>

#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"

enum
{
    // Service info
    DEVICE_INFO_SERVICE_SVC,

    // System ID Declaration
    DEVICE_INFO_SERVICE_SYSTEM_ID_CHAR,
    // System ID Value
    DEVICE_INFO_SERVICE_SYSTEM_ID_VAL,

    // Model Number String Declaration
    DEVICE_INFO_SERVICE_MODEL_NUMBER_STR_CHAR,
    // Model Number String Value
    DEVICE_INFO_SERVICE_MODEL_NUMBER_STR_VAL,

    // Serial Number String Declaration
    DEVICE_INFO_SERVICE_SERIAL_NUMBER_STR_CHAR,
    // Serial Number String Value
    DEVICE_INFO_SERVICE_SERIAL_NUMBER_STR_VAL,

    // Firmware Revision String Declaration
    DEVICE_INFO_SERVICE_FIRMWARE_VERSION_STR_CHAR,
    // Firmware Revision String Value
    DEVICE_INFO_SERVICE_FIRMWARE_VERSION_STR_VAL,

    // Hardware Revision String Declaration
    DEVICE_INFO_SERVICE_HARDWARE_VERSION_STR_CHAR,
    // Hardware Revision String Value
    DEVICE_INFO_SERVICE_HARDWARE_VERSION_STR_VAL,

    // Software Revision String Declaration
    DEVICE_INFO_SERVICE_SOFTWARE_VERSION_STR_CHAR,
    // Software Revision String Value
    DEVICE_INFO_SERVICE_SOFTWARE_VERSION_STR_VAL,

    // Manufacturer Name String Declaration
    DEVICE_INFO_SERVICE_MANUFACTURER_NAME_CHAR,
    // Manufacturer Name String Value
    DEVICE_INFO_SERVICE_MANUFACTURER_NAME_VAL,

    // IEEE 11073-20601 Regulatory Certification Data List Declaration
    DEVICE_INFO_SERVICE_IEEE_DATA_CHAR,
    // IEEE 11073-20601 Regulatory Certification Data List Value
    DEVICE_INFO_SERVICE_IEEE_DATA_VAL,

    // PnP ID Declaration
    DEVICE_INFO_SERVICE_PNP_ID_CHAR,
    // PnP ID Value
    DEVICE_INFO_SERVICE_PNP_ID_VAL,

    DEVICE_INFO_SERVICE_NUM_ATTR
};

extern uint16_t device_info_service_handle_table[];
extern const uint16_t uuid_DEVICE_INFO_SERVICE_SVC;
extern const esp_gatts_attr_db_t device_info_service_gatt_db[DEVICE_INFO_SERVICE_NUM_ATTR];
extern const ble_gatt_server_service_t device_info_service;