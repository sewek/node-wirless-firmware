#pragma once

#include <esp_err.h>

#define PROFILE_NUM 1
#define PROFILE_APP_IDX 0
#define ESP_APP_ID 0x55

esp_err_t ble_gatt_server_init(void);
esp_err_t ble_gatt_server_deinit(void);