#pragma once

#include "esp_err.h"

#define SENSOR_RESULT_SIZE sizeof(uint16_t)

enum
{
    SENSOR_STATUS_OK,
    SENSOR_STATUS_NOT_READY,
    SENSOR_STATUS_HALTED,
    SENSOR_STATUS_ERROR,
    SENSOR_STATUS_BUSY,
    SENSOR_STATUS_TIMEOUT,
    SENSOR_STATUS_NOT_SUPPORTED,
    SENSOR_STATUS_NOT_ENOUGH_MEM,
};

typedef struct
{
    uint8_t alert;
    uint8_t config;
    uint8_t low_limit;
    uint8_t high_limit;
    uint8_t hysteresis;
} sensor_config_t;

extern uint8_t sensor_status;
extern sensor_config_t sensor_config;

esp_err_t sensor_interface_init(void);
esp_err_t sensor_interface_deinit(void);

esp_err_t sensor_read_result(uint16_t *);
esp_err_t sensor_set_config(sensor_config_t *);