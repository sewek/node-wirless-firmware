#include "sensor_service.h"
#include "sensor_task.h"
#include "sensor_interface.h"

#include <string.h>

#include "esp_gatts_api.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define TAG "BT SENSOR SERVICE"

// TODO: Remove this after implementation checking task

uint16_t sensor_service_handle_table[SENSOR_SERVICE_NUM_ATTR];

const uint8_t uuid_SENSOR_SERVICE_SVC[ESP_UUID_LEN_128] = {0xBA, 0xC9, 0x76, 0x83, 0xCD, 0xE3, 0x4E, 0x6C, 0x9E, 0xF8, 0x59, 0xC7, 0x88, 0x0D, 0xFF, 0xD1};

/* UUIDs */
/* Info         0x0000 - 0x9f00 */
const uint16_t uuid_SENSOR_SERVICE_STATUS_CHAR = 0x0000;
const uint16_t uuid_SENSOR_SERVICE_MAX_FRAMES_VALUE = 0x0001;
const uint16_t uuid_SENSOR_SERVICE_FRAMES_COUNT_VALUE = 0x0002;
const uint16_t uuid_SENSOR_SERVICE_MAX_SAMPLES_VALUE = 0x0003;
const uint16_t uuid_SENSOR_SERVICE_SAMPLES_COUNT_VALUE = 0x0004;

/* Controlling  0xa000 - 0xe900 */
const uint16_t uuid_SENSOR_SERVICE_ENABLE_VALUE = 0xa000;

/* Data         0xf000 - 0xffff */
const uint16_t uuid_SENSOR_SERVICE_FRAME_VALUE = 0xf000;
/* End UUIDs */

/* DESCRIPTIONS */
static uint8_t description_sensor_status[] = "Sensor status 0x00: ok, 0x01: not ready, 0x02: halted, 0x03: error, 0x04: busy, 0x05: timeout, 0x06: not supported, 0x07: not enough memory";
static uint8_t description_sensor_enable[] = "Sensor enable control 0x00: disable, 0x01: enable";
static uint8_t description_sensor_frame[] = "Sensor frame data";
/* END DESCRIPTIONS */

/* CONFIGURATIONS */
static uint8_t configuration_sensor_status[] = {0x00, 0x01};
static uint8_t configuration_sensor_enable[] = {0x00, 0x01};
static uint8_t configuration_sensor_frame[] = {0x00, 0x00};
/* END CONFIGURATIONS */

/* VALUES */
/* Info */
static uint8_t value_sensor_status[1] = {0x00};
static uint8_t value_sensor_max_frames_per_data[1] = {SENSOR_MAX_FRAMES_PER_DATA};
static uint8_t value_sensor_frames_count[1] = {0x00};
static uint8_t value_sensor_max_samples_per_frame[1] = {SENSOR_MAX_SAMPLES_PER_FRAME};
static uint8_t value_sensor_samples_count[SENSOR_MAX_FRAMES_PER_DATA] = {0x00};

/* Controlling */
static uint8_t value_sensor_enable[1] = {0x00};

/* Data */
static uint16_t value_sensor_frame[SENSOR_MAX_SAMPLES_PER_FRAME] = {0x00};
/* END VALUES */

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_description_uuid = ESP_GATT_UUID_CHAR_DESCRIPTION;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

const esp_gatts_attr_db_t sensor_service_gatt_db[SENSOR_SERVICE_NUM_ATTR] = {
    [SENSOR_SERVICE_SVC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uuid_SENSOR_SERVICE_SVC), (uint8_t *)&uuid_SENSOR_SERVICE_SVC}},

    /* Info */
    [SENSOR_SERVICE_STATUS_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_notify}},
    [SENSOR_SERVICE_STATUS_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_STATUS_CHAR, ESP_GATT_PERM_READ, sizeof(value_sensor_status), sizeof(value_sensor_status), (uint8_t *)value_sensor_status}},
    [SENSOR_SERVICE_STATUS_DESC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_description_uuid, ESP_GATT_PERM_READ, sizeof(description_sensor_status), sizeof(description_sensor_status), (uint8_t *)description_sensor_status}},
    [SENSOR_SERVICE_STATUS_CFG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ, sizeof(configuration_sensor_status), sizeof(configuration_sensor_status), (uint8_t *)configuration_sensor_status}},

    [SENSOR_SERVICE_MAX_FRAMES_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_MAX_FRAMES_VALUE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_MAX_FRAMES_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_max_frames_per_data), sizeof(value_sensor_max_frames_per_data), (uint8_t *)value_sensor_max_frames_per_data}},

    [SENSOR_SERVICE_FRAMES_COUNT_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_FRAMES_COUNT_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_FRAMES_COUNT_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_frames_count), sizeof(value_sensor_frames_count), (uint8_t *)value_sensor_frames_count}},

    [SENSOR_SERVICE_MAX_SAMPLES_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_MAX_SAMPLES_VALUE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_MAX_SAMPLES_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_max_samples_per_frame), sizeof(value_sensor_max_samples_per_frame), (uint8_t *)value_sensor_max_samples_per_frame}},

    [SENSOR_SERVICE_SAMPLES_COUNT_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read}},
    [SENSOR_SERVICE_SAMPLES_COUNT_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_SAMPLES_COUNT_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_samples_count), sizeof(value_sensor_samples_count), (uint8_t *)value_sensor_samples_count}},

    /* Controlling */
    [SENSOR_SERVICE_ENABLE_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_write_notify}},
    [SENSOR_SERVICE_ENABLE_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_ENABLE_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_enable), sizeof(value_sensor_enable), (uint8_t *)value_sensor_enable}},
    [SENSOR_SERVICE_ENABLE_DESC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_description_uuid, ESP_GATT_PERM_READ, sizeof(description_sensor_enable), sizeof(description_sensor_enable), (uint8_t *)description_sensor_enable}},
    [SENSOR_SERVICE_ENABLE_CFG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(configuration_sensor_enable), sizeof(configuration_sensor_enable), (uint8_t *)&configuration_sensor_enable}},

    /* Data */
    [SENSOR_SERVICE_FRAME_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_notify}},
    [SENSOR_SERVICE_FRAME_VALUE] = {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SENSOR_SERVICE_FRAME_VALUE, ESP_GATT_PERM_READ, sizeof(value_sensor_frame), sizeof(value_sensor_frame), (uint8_t *)value_sensor_frame}},
    [SENSOR_SERVICE_FRAME_DESC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_description_uuid, ESP_GATT_PERM_READ, sizeof(description_sensor_frame), sizeof(description_sensor_frame), (uint8_t *)description_sensor_frame}},
    [SENSOR_SERVICE_FRAME_CFG] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(configuration_sensor_frame), sizeof(configuration_sensor_frame), (uint8_t *)configuration_sensor_frame}},
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
    case SENSOR_SERVICE_STATUS_VALUE:

        value_sensor_status[0] = sensor_status;

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, value_sensor_status, sizeof(value_sensor_status));
        rsp->attr_value.len = sizeof(value_sensor_status);

        break;

    case SENSOR_SERVICE_FRAMES_COUNT_VALUE:

        value_sensor_frames_count[0] = sensor_data.frames_count;

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, value_sensor_frames_count, sizeof(value_sensor_frames_count));
        rsp->attr_value.len = sizeof(value_sensor_frames_count);

        break;

    case SENSOR_SERVICE_SAMPLES_COUNT_VALUE:
        for (int i = 0; i < sensor_data.frames_count; i++)
        {
            value_sensor_samples_count[i] = sensor_data.frames[i].samples_count;
        }

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, value_sensor_samples_count, sizeof(value_sensor_samples_count));
        rsp->attr_value.len = sizeof(value_sensor_samples_count);

        break;

    case SENSOR_SERVICE_FRAME_VALUE:
        uint8_t frame_index = sensor_data.current_frame;

        if (frame_index == 0)
        {
            frame_index = sensor_data.frames_count - 1;
        }

        memset(value_sensor_frame, 0, sizeof(value_sensor_frame));
        memcpy(value_sensor_frame, sensor_data.frames[frame_index].samples, sizeof(sensor_data.frames[frame_index].samples));

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, value_sensor_frame, sizeof(value_sensor_frame));
        rsp->attr_value.len = sizeof(value_sensor_frame);

        break;

    case SENSOR_SERVICE_ENABLE_VALUE:
        if (sensor_status == SENSOR_STATUS_HALTED)
        {
            value_sensor_enable[0] = 0;
        }
        else
        {
            value_sensor_enable[0] = 1;
        }

        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, value_sensor_enable, sizeof(value_sensor_enable));
        rsp->attr_value.len = sizeof(value_sensor_enable);

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