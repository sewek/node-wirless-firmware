#pragma once

#include "esp_err.h"

#define SENSOR_SAMPLING_RATE 400
#define SENSOR_MAX_FRAMES_PER_DATA 10
#define SENSOR_MAX_SAMPLES_PER_FRAME 100

typedef struct
{
    uint8_t samples_count;
    uint8_t samples_size;
    uint16_t samples_avg_delay;

    uint16_t samples[SENSOR_MAX_SAMPLES_PER_FRAME];
} sensor_data_frame_t;

typedef struct
{
    uint8_t frames_count;
    uint8_t current_frame;
    sensor_data_frame_t frames[SENSOR_MAX_FRAMES_PER_DATA];
} sensor_data_t;

extern sensor_data_t sensor_data;

esp_err_t sensor_task_init(void);
esp_err_t sensor_task_deinit(void);