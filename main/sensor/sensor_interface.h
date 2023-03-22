#pragma once

#include "esp_err.h"

#define SENSOR_RESULT_SIZE sizeof(uint16_t)

typedef struct
{
    uint8_t alert;
    uint8_t config;
    uint8_t low_limit;
    uint8_t high_limit;
    uint8_t hysteresis;
} sensor_config_t;

extern sensor_config_t sensor_config;

esp_err_t sensor_interface_init(void);
esp_err_t sensor_interface_deinit(void);

esp_err_t sensor_read_result(uint16_t *);
esp_err_t sensor_set_config(sensor_config_t *);