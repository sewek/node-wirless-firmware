#include "sensor_interface.h"
#include "sensor_i2c.h"
#include "sensor_task.h"

#include "esp_log.h"

#include "sdkconfig.h"

#define TAG "SENSOR_INTERFACE"

sensor_config_t sensor_config = {
    .alert = 0x00,
    .config = (0x07 << 5), // 12-bit conversion, 400Hz conversion rate, comparator mode
    .low_limit = 0x00,
    .high_limit = 0x00,
    .hysteresis = 0x00,
};

esp_err_t sensor_interface_init()
{
    esp_err_t ret;

    // Initialize the sensor
    ret = sensor_i2c_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensor with error code: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Initialized sensor.");

    // Set the sensor configuration
    ret = sensor_set_config(&sensor_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set sensor configuration with error code: %d", ret);
        return ret;
    }

    // Initialize the sensor task
    ret = sensor_task_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sensor task with error code: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Initialized sensor task.");

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