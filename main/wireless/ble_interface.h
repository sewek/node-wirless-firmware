#pragma once

#include <stdint.h>
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_err.h"

typedef struct
{
    uint16_t len;
    union
    {
        uint16_t *uuid16;
        uint32_t *uuid32;
        uint8_t *uuid128;
    } uuid;
} ble_gatt_uuid_pointers_t;

typedef uint16_t (*gatts_service_get_attribute_cb_t)(uint16_t attr_index);
typedef esp_err_t (*gatts_service_read_event_cb_t)(int attr_index, esp_ble_gatts_cb_param_t *param, esp_gatt_rsp_t *rsp);
typedef esp_err_t (*gatts_service_write_event_cb_t)(int attr_index, esp_ble_gatts_cb_param_t *param, esp_gatt_rsp_t *rsp);

typedef struct ble_gatt_server_service
{
    ble_gatt_uuid_pointers_t uuid;
    uint8_t service_uuid_index;
    uint16_t *table_handle;
    esp_gatts_attr_db_t *gatt_db;
    uint16_t gatt_db_len;

    gatts_service_get_attribute_cb_t get_attribute;
    gatts_service_read_event_cb_t read_event_cb;
    gatts_service_write_event_cb_t write_event_cb;
} ble_gatt_server_service_t;

esp_err_t ble_interface_init(void);
esp_err_t ble_interface_deinit(void);