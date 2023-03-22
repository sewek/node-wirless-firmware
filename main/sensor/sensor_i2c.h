#pragma once

#include "esp_err.h"

#define SENSOR_REG_RESULT 0x00
#define SENSOR_REG_ALERT 0x01
#define SENSOR_REG_CONFIG 0x02
#define SENSOR_REG_LOW_LIMIT 0x03
#define SENSOR_REG_HIGH_LIMIT 0x04
#define SENSOR_REG_HYSTERESIS 0x05
#define SENSOR_REG_LOWEST_CONV 0x06
#define SENSOR_REG_HIGHEST_CONV 0x07

esp_err_t sensor_i2c_init(void);
esp_err_t sensor_i2c_deinit(void);

esp_err_t sensor_i2c_read(uint8_t, uint8_t *, size_t);
esp_err_t sensor_i2c_write(uint8_t, uint8_t);