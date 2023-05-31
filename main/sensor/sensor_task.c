#include "sensor_task.h"
#include "sensor_i2c.h"
#include "sensor_interface.h"

#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "sdkconfig.h"

#define TAG "SENSOR_TASK"

sensor_data_t sensor_data = {
    .current_frame = 0,
    .frames_count = SENSOR_MAX_FRAMES_PER_DATA,
};

static SemaphoreHandle_t sensor_data_mutex = NULL;
static TaskHandle_t sensor_task_handle = NULL;

static uint32_t get_current_time_ms(void)
{
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

static void sensor_task(void *pvParameters)
{
    esp_err_t ret;
    uint32_t start_time = 0, end_time = 0, duration = 0, delays_sum = 0;
    uint8_t samples_count = 0;
    uint16_t data = 0x00;

    xSemaphoreTake(sensor_data_mutex, portMAX_DELAY);

    ESP_LOGI(TAG, "Sensor task running");
    while (1)
    {
        start_time = get_current_time_ms();

        if (end_time != 0)
        {
            delays_sum += start_time - end_time;
        }

        ret = sensor_read_result(&data);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read sensor");
            data = 0;
        }

        samples_count = sensor_data.frames[sensor_data.current_frame].samples_count;            // Get the current sample count
        sensor_data.frames[sensor_data.current_frame].samples[samples_count] = (data & 0x0FFF); // Add the sample to the current frame

        if (samples_count >= SENSOR_MAX_SAMPLES_PER_FRAME - 1) // If the current frame is full
        {
            sensor_data.frames[sensor_data.current_frame].samples_size = sizeof(uint16_t);                // Set the sample size
            sensor_data.frames[sensor_data.current_frame].samples_avg_delay = delays_sum / samples_count; // Set the average delay
            delays_sum = 0;                                                                               // Reset the delays sum

            if (sensor_data.frames_count < SENSOR_MAX_FRAMES_PER_DATA) // If the frames count is less than the max frames per data
            {
                sensor_data.frames_count++; // Increment the frames count
            }

            sensor_data.current_frame++;                                 // Increment the current frame
            if (sensor_data.current_frame >= SENSOR_MAX_FRAMES_PER_DATA) // If the current frame is the last frame
            {
                sensor_data.current_frame = 0; // Reset the current frame
            }
            sensor_data.frames[sensor_data.current_frame].samples_count = 0; // Reset the sample count
        }
        else // If the current frame is not full (has space)
        {
            sensor_data.frames[sensor_data.current_frame].samples_count++; // Increment the sample count
        }

        end_time = get_current_time_ms();
        duration = end_time - start_time;

        xSemaphoreGive(sensor_data_mutex);

        vTaskDelay((((float)SENSOR_SAMPLING_RATE / 1000) - duration) / portTICK_PERIOD_MS);
    }
}

esp_err_t sensor_task_init(void)
{
    // Create the mutex
    sensor_data_mutex = xSemaphoreCreateBinary();

    // Create the task
    xTaskCreate(sensor_task, "Sensor Task", 4096, NULL, 5, &sensor_task_handle);
    xSemaphoreGive(sensor_data_mutex);

    return ESP_OK;
}

esp_err_t sensor_task_deinit(void)
{
    // Delete the task
    if (sensor_task_handle != NULL)
    {
        vTaskDelete(sensor_task_handle);
        sensor_task_handle = NULL;
    }

    return ESP_OK;
}