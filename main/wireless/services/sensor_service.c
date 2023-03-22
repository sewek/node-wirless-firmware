#include "sensor_service.h"
#include "sensor_task.h"

#include <string.h>

#include "esp_gatts_api.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define TAG "BT SENSOR SERVICE"

// TODO: Remove this after implementation checking task

uint16_t sensor_service_handle_table[SENSOR_SERVICE_NUM_ATTR];

const uint8_t uuid_SENSOR_SERVICE_SVC[ESP_UUID_LEN_128] = {0xBA, 0xC9, 0x76, 0x83, 0xCD, 0xE3, 0x4E, 0x6C, 0x9E, 0xF8, 0x59, 0xC7, 0x88, 0x0D, 0xFF, 0xD1};
const uint16_t uuid_SENSOR_SERVICE_MAX_FRAMES_VALUE = 0x0000;
const uint16_t uuid_SENSOR_SERVICE_FRAMES_COUNT_VALUE = 0x0001;
const uint16_t uuid_SENSOR_SERVICE_MAX_SAMPLES_VALUE = 0x0002;
const uint16_t uuid_SENSOR_SERVICE_SAMPLES_COUNT_VALUE = 0x0003;
const uint16_t uuid_SENSOR_SERVICE_FRAME_VALUE = 0x0004;

static uint8_t sensor_frame_desc[] = "Sensor frame data";
static uint8_t sensor_frame_cfg[] = {0x00, 0x00};

static uint8_t sensor_level[] = {69};
static uint8_t sensor_max_frames_per_data[1] = {SENSOR_MAX_FRAMES_PER_DATA};
static uint8_t sensor_frames_count[1] = {0x00};
static uint8_t sensor_max_samples_per_frame[1] = {SENSOR_MAX_SAMPLES_PER_FRAME};
static uint8_t sensor_samples_count[SENSOR_MAX_FRAMES_PER_DATA] = {0x00};
static uint16_t sensor_frame[SENSOR_MAX_SAMPLES_PER_FRAME] = {0x00};

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_description_uuid = ESP_GATT_UUID_CHAR_DESCRIPTION;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

const esp_gatts_attr_db_t sensor_service_gatt_db[SENSOR_SERVICE_NUM_ATTR] = {
    [SENSOR_SERVICE_SVC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uuid_SENSOR_SERVICE_SVC), (uint8_t *)&uuid_SENSOR_SERVICE_SVC}},

    [SENSOR_SERVICE_MAX_FRAMES_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_MAX_FRAMES_VALUE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_MAX_FRAMES_VALUE, ESP_GATT_PERM_READ, sizeof(sensor_max_frames_per_data), sizeof(sensor_max_frames_per_data), (uint8_t *)sensor_max_frames_per_data}},

    [SENSOR_SERVICE_FRAMES_COUNT_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_FRAMES_COUNT_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_FRAMES_COUNT_VALUE, ESP_GATT_PERM_READ, sizeof(sensor_frames_count), sizeof(sensor_frames_count), (uint8_t *)sensor_frames_count}},

    [SENSOR_SERVICE_MAX_SAMPLES_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_MAX_SAMPLES_VALUE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_MAX_SAMPLES_VALUE, ESP_GATT_PERM_READ, sizeof(sensor_max_samples_per_frame), sizeof(sensor_max_samples_per_frame), (uint8_t *)sensor_max_samples_per_frame}},

    [SENSOR_SERVICE_SAMPLES_COUNT_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_SAMPLES_COUNT_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_SAMPLES_COUNT_VALUE, ESP_GATT_PERM_READ, sizeof(sensor_samples_count), sizeof(sensor_samples_count), (uint8_t *)sensor_samples_count}},

    [SENSOR_SERVICE_FRAME_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_notify}},
    [SENSOR_SERVICE_FRAME_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_FRAME_VALUE, ESP_GATT_PERM_READ, sizeof(sensor_frame), sizeof(sensor_frame), (uint8_t *)sensor_frame}},
    [SENSOR_SERVICE_FRAME_DESC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_description_uuid, ESP_GATT_PERM_READ, sizeof(sensor_frame_desc), sizeof(sensor_frame_desc), (uint8_t *)sensor_frame_desc}},
    [SENSOR_SERVICE_FRAME_CFG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(sensor_frame_cfg), sizeof(sensor_frame_cfg), (uint8_t *)sensor_frame_cfg}},
};

static uint16_t get_attribute(uint16_t handle)
{
    for (int i = 0; i < SENSOR_SERVICE_NUM_ATTR; i++)
    {
        if (sensor_service_handle_table[i] == handle)
        {
            return i;
        }
    }

    return SENSOR_SERVICE_NUM_ATTR;
}

static esp_err_t read_event_handler(int attr_index, esp_ble_gatts_cb_param_t *param, esp_gatt_rsp_t *rsp)
{
    ESP_LOGI(TAG, "Read event handler for attribute: %d", attr_index);

    switch (attr_index)
    {
    case SENSOR_SERVICE_FRAMES_COUNT_VALUE:

        sensor_frames_count[0] = sensor_data.frames_count;

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, sensor_frames_count, sizeof(sensor_frames_count));
        rsp->attr_value.len = sizeof(sensor_frames_count);

        break;

    case SENSOR_SERVICE_SAMPLES_COUNT_VALUE:
        for (int i = 0; i < sensor_data.frames_count; i++)
        {
            sensor_samples_count[i] = sensor_data.frames[i].samples_count;
        }

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, sensor_samples_count, sizeof(sensor_samples_count));
        rsp->attr_value.len = sizeof(sensor_samples_count);

        break;

    case SENSOR_SERVICE_FRAME_VALUE:
        uint8_t frame_index = sensor_data.current_frame;

        if (frame_index == 0)
        {
            frame_index = sensor_data.frames_count - 1;
        }

        memset(sensor_frame, 0, sizeof(sensor_frame));
        memcpy(sensor_frame, sensor_data.frames[frame_index].samples, sizeof(sensor_data.frames[frame_index].samples));

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, sensor_frame, sizeof(sensor_frame));
        rsp->attr_value.len = sizeof(sensor_frame);

        break;
    }

    return ESP_OK;
}

const ble_gatt_server_service_t sensor_service = {
    .service_uuid_index = SENSOR_SERVICE_SVC,
    .table_handle = sensor_service_handle_table,
    .gatt_db = sensor_service_gatt_db,
    .gatt_db_len = SENSOR_SERVICE_NUM_ATTR,

    .get_attribute = get_attribute,
    .read_event_cb = read_event_handler,

    .uuid = {
        .len = ESP_UUID_LEN_128,
        .uuid = {
            .uuid16 = uuid_SENSOR_SERVICE_SVC,
        },
    },
};