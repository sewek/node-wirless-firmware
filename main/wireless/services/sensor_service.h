#ifndef __SENSOR_SERVICE_H__
#define __SENSOR_SERVICE_H__

#include "ble_interface.h"

#include <stdint.h>

#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"

enum
{
    SENSOR_SERVICE_SVC,

    /* Info characteristics */

    SENSOR_SERVICE_STATUS_CHAR,
    SENSOR_SERVICE_STATUS_VALUE,
    SENSOR_SERVICE_STATUS_DESC,
    SENSOR_SERVICE_STATUS_CFG,

    SENSOR_SERVICE_MAX_FRAMES_CHAR,
    SENSOR_SERVICE_MAX_FRAMES_VALUE,

    SENSOR_SERVICE_FRAMES_COUNT_CHAR,
    SENSOR_SERVICE_FRAMES_COUNT_VALUE,

    SENSOR_SERVICE_MAX_SAMPLES_CHAR,
    SENSOR_SERVICE_MAX_SAMPLES_VALUE,

    SENSOR_SERVICE_SAMPLES_COUNT_CHAR,
    SENSOR_SERVICE_SAMPLES_COUNT_VALUE,

    /* Controling characteristics */

    SENSOR_SERVICE_ENABLE_CHAR,
    SENSOR_SERVICE_ENABLE_VALUE,
    SENSOR_SERVICE_ENABLE_DESC,
    SENSOR_SERVICE_ENABLE_CFG,

    /* Streamming characteristics */

    SENSOR_SERVICE_FRAME_CHAR,
    SENSOR_SERVICE_FRAME_VALUE,
    SENSOR_SERVICE_FRAME_DESC,
    SENSOR_SERVICE_FRAME_CFG,

    /* Length */

    SENSOR_SERVICE_NUM_ATTR,
};

extern uint16_t sensor_service_handle_table[];
extern const uint8_t uuid_SENSOR_SERVICE_SVC[];
extern const esp_gatts_attr_db_t sensor_service_gatt_db[SENSOR_SERVICE_NUM_ATTR];
extern const ble_gatt_server_service_t sensor_service;

#endif // __SENSOR_SERVICE_H__