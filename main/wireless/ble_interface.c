
#include "ble_interface.h"
#include "ble_gap.h"
#include "ble_gatt_server.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#include "sdkconfig.h"

#define TAG "BLE INTERFACE"

esp_err_t ble_interface_init(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initializing Bluetooth Driver");

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(TAG, "initialize controller failed");
        return ret;
    }

    ret = esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N0);
    if (ret)
    {
        ESP_LOGE(TAG, "set tx power failed, error code = %x", ret);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(TAG, "enable controller failed");
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(TAG, "init bluetooth failed");
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(TAG, "enable bluetooth failed");
        return ret;
    }

    ret = ble_gap_init();
    if (ret)
    {
        ESP_LOGE(TAG, "ble gap init failed, error code = %x", ret);
        return ret;
    }

    ret = ble_gatt_server_init();
    if (ret)
    {
        ESP_LOGE(TAG, "ble gatt server init failed, error code = %x", ret);
        return ret;
    }

    ESP_LOGI(TAG, "Bluetooth Driver Initialized");

    return ret;
}

esp_err_t ble_interface_deinit(void)
{
    esp_err_t ret = ESP_OK;

    ret = ble_gatt_server_deinit();
    if (ret)
    {
        ESP_LOGE(TAG, "ble gatt server deinit failed, error code = %x", ret);
        return ret;
    }

    ret = ble_gap_deinit();
    if (ret)
    {
        ESP_LOGE(TAG, "ble gap deinit failed, error code = %x", ret);
        return ret;
    }

    ret = esp_bluedroid_disable();
    if (ret)
    {
        ESP_LOGE(TAG, "disable bluedroid failed");
        return ret;
    }

    ret = esp_bluedroid_deinit();
    if (ret)
    {
        ESP_LOGE(TAG, "deinitialize bluedroid failed");
        return ret;
    }

    ret = esp_bt_controller_disable();
    if (ret)
    {
        ESP_LOGE(TAG, "disable bt controller failed");
        return ret;
    }

    ret = esp_bt_controller_deinit();
    if (ret)
    {
        ESP_LOGE(TAG, "deinitialize bt controller failed");
        return ret;
    }

    return ret;
}