#pragma once

#include "esp_err.h"

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)

esp_err_t ble_gap_init(void);
esp_err_t ble_gap_deinit(void);
esp_err_t ble_gap_set_advertising_data(void);
esp_err_t ble_gap_start_advertising(void);
esp_err_t ble_gap_stop_advertising(void);
