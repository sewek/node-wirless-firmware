#include "sensor_i2c.h"

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "sdkconfig.h"

#define TAG "SENSOR_I2C"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_FREQ_HZ 100000

#define I2C_SENSOR_ADDR 0b1010000 // 0x28

esp_err_t sensor_i2c_init()
{
    esp_err_t ret;

    // Initialize the I2C bus
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ret = i2c_param_config(I2C_MASTER_NUM, &i2c_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure I2C parameters with error code: %d", ret);
        return ret;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, i2c_config.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install I2C driver with error code: %d", ret);
        return ret;
    }

    return ESP_OK;
}

esp_err_t sensor_i2c_deinit(void)
{
    esp_err_t ret;

    ret = i2c_driver_delete(I2C_MASTER_NUM);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete I2C driver with error code: %d", ret);
        return ret;
    }

    return ESP_OK;
}

esp_err_t sensor_i2c_read(uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, I2C_SENSOR_ADDR, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

esp_err_t sensor_i2c_write(uint8_t reg, uint8_t data)
{
    esp_err_t ret;
    uint8_t buff[2] = {reg, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, I2C_SENSOR_ADDR, buff, sizeof(buff), 1000 / portTICK_PERIOD_MS);

    return ret;
}