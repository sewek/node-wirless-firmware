/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

/****************************************************************************
 *
 * This demo showcases BLE GATT server. It can send adv data, be connected by client.
 * Run the gatt_client demo, the client demo will automatically connect to the gatt_server demo.
 * Client demo will enable gatt_server's notify after connection. The two devices will then exchange
 * data.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "wireless/ble_interface.h"
#include "sensor/sensor_interface.h"

#include "sdkconfig.h"

#define TAG "GATTS_DEMO"

void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "Initialized FLASH.");

    // Initialize the sensor
    ret = sensor_interface_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensor");
        return;
    }
    ESP_LOGI(TAG, "Initialized Sensor.");

    // Initialize Bluetooth
    ret = ble_interface_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Bluetooth");
        return;
    }
    ESP_LOGI(TAG, "Initialized Bluetooth.");

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "====================");
    ESP_LOGI(TAG, "Ready!");
    ESP_LOGI(TAG, "====================");
    ESP_LOGI(TAG, "");

    ESP_LOGI(TAG, "Entering infinite loop...");
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    return;
}