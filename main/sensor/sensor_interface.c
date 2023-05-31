#include "sensor_interface.h"

#include <stdbool.h>

#include "sensor_i2c.h"
#include "sensor_task.h"

#include "esp_log.h"

#include "sdkconfig.h"

#define TAG "SENSOR_INTERFACE"

uint8_t sensor_status = SENSOR_STATUS_HALTED;
sensor_config_t sensor_config = {
    .alert = 0x00,
    .config = 0x00,
    .low_limit = 0x00,
    .high_limit = 0x00,
    .hysteresis = 0x00,
};

esp_err_t sensor_interface_init()
{
    esp_err_t ret;

    sensor_status = SENSOR_STATUS_NOT_READY;

    // Initialize the sensor
    ret = sensor_i2c_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensor with error code: %d", ret);
        sensor_status = SENSOR_STATUS_HALTED;
        return ret;
    }
    ESP_LOGI(TAG, "Initialized sensor.");

    sensor_config.alert = 0x00;
    sensor_config.config = (0x07 << 5); // 12-bit conversion, 400Hz conversion rate, comparator mode
    sensor_config.low_limit = 0x00;
    sensor_config.high_limit = 0x00;
    sensor_config.hysteresis = 0x00;

    // Set the sensor configuration
    ret = sensor_set_config(&sensor_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set sensor configuration with error code: %d", ret);
        sensor_status = SENSOR_STATUS_HALTED;
        return ret;
    }

    // Initialize the sensor task
    ret = sensor_task_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensor task with error code: %d", ret);
        sensor_status = SENSOR_STATUS_HALTED;
        return ret;
    }
    ESP_LOGI(TAG, "Initialized sensor task.");

    sensor_status = SENSOR_STATUS_OK;

    return ESP_OK;
}

esp_err_t sensor_interface_deinit(void)
{
    esp_err_t ret;

    ret = sensor_task_deinit();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to deinitialize sensor task with error code: %d", ret);
        sensor_status = SENSOR_STATUS_ERROR;
        return ret;
    }
    ESP_LOGI(TAG, "Deinitialized sensor task.");

    sensor_config.alert = 0x00;
    sensor_config.config = 0x00;
    sensor_config.low_limit = 0x00;
    sensor_config.high_limit = 0x00;
    sensor_config.hysteresis = 0x00;

    ret = sensor_set_config(&sensor_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set sensor configuration with error code: %d", ret);
        sensor_status = SENSOR_STATUS_ERROR;
        return ret;
    }

    ret = sensor_i2c_deinit();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to deinitialize sensor with error code: %d", ret);
        sensor_status = SENSOR_STATUS_ERROR;
        return ret;
    }
    ESP_LOGI(TAG, "Deinitialized sensor.");

    sensor_status = SENSOR_STATUS_HALTED;

    return ESP_OK;
}

esp_err_t sensor_read_result(uint16_t *data)
{
    return sensor_i2c_read(SENSOR_REG_RESULT, data, sizeof(*data));
}

esp_err_t sensor_set_config(sensor_config_t *data)
{
    esp_err_t ret;

    ret = sensor_i2c_write(SENSOR_REG_ALERT, data->alert);
    ret = sensor_i2c_write(SENSOR_REG_CONFIG, data->config);
    ret = sensor_i2c_write(SENSOR_REG_LOW_LIMIT, data->low_limit);
    ret = sensor_i2c_write(SENSOR_REG_HIGH_LIMIT, data->high_limit);
    ret = sensor_i2c_write(SENSOR_REG_HYSTERESIS, data->hysteresis);

    return ret;
}