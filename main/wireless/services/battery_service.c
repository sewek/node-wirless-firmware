#include "battery_service.h"

#include <string.h>

#include "esp_gatts_api.h"
#include "esp_log.h"

#define TAG "BT BATTERY SERVICE"

// TODO: Remove this after implementation checking task

uint16_t battery_service_handle_table[BATTERY_SERVICE_NUM_ATTR];

const uint16_t uuid_BATTERY_SERVICE_SVC = ESP_GATT_UUID_BATTERY_SERVICE_SVC;
const uint16_t uuid_BATTERY_SERVICE_VALUE = ESP_GATT_UUID_BATTERY_LEVEL;
const uint16_t uuid_BATTERY_SERVICE_DESC = ESP_GATT_UUID_CHAR_DESCRIPTION;
const uint16_t uuid_BATTERY_SERVICE_CONF = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static uint8_t battery_level[] = {69};
static const uint8_t BATTERY_SERVICE_DATA_descriptor[] = "Percentage 0 - 100";
static const uint8_t BATTERY_SERVICE_DATA_client_config[] = {0x00, 0x00};

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_description_uuid = ESP_GATT_UUID_CHAR_DESCRIPTION;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
// static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

const esp_gatts_attr_db_t battery_service_gatt_db[BATTERY_SERVICE_NUM_ATTR] = {
    [BATTERY_SERVICE_SVC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uuid_BATTERY_SERVICE_SVC), (uint8_t *)&uuid_BATTERY_SERVICE_SVC}},
    [BATTERY_SERVICE_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(uint8_t), (uint8_t *)&char_prop_read_notify}},
    [BATTERY_SERVICE_VALUE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_BATTERY_SERVICE_VALUE, ESP_GATT_PERM_READ, sizeof(battery_level), sizeof(battery_level), (uint8_t *)&battery_level}},
    [BATTERY_SERVICE_DESC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_description_uuid, ESP_GATT_PERM_READ, sizeof(BATTERY_SERVICE_DATA_descriptor), sizeof(BATTERY_SERVICE_DATA_descriptor), (uint8_t *)&BATTERY_SERVICE_DATA_descriptor}},
    [BATTERY_SERVICE_CONF] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(BATTERY_SERVICE_DATA_client_config), (uint8_t *)&BATTERY_SERVICE_DATA_client_config}},
};

static uint16_t get_attribute(uint16_t handle)
{
    for (int i = 0; i < BATTERY_SERVICE_NUM_ATTR; i++)
    {
        if (battery_service_handle_table[i] == handle)
        {
            return i;
        }
    }

    return BATTERY_SERVICE_NUM_ATTR;
}

static esp_err_t read_event_handler(int attr_index, esp_ble_gatts_cb_param_t *param, esp_gatt_rsp_t *rsp)
{
    ESP_LOGI(TAG, "Read event handler");

    switch (attr_index)
    {
    case BATTERY_SERVICE_VALUE:
        memset(rsp->attr_value.value, 0, sizeof(rsp->attr_value.value));
        memcpy(rsp->attr_value.value, battery_level, sizeof(battery_level));
        rsp->attr_value.len = sizeof(battery_level);
        break;
    }

    return ESP_OK;
}

const ble_gatt_server_service_t battery_service = {
    .service_uuid_index = BATTERY_SERVICE_SVC,
    .table_handle = &battery_service_handle_table,
    .gatt_db = &battery_service_gatt_db,
    .gatt_db_len = BATTERY_SERVICE_NUM_ATTR,

    .get_attribute = get_attribute,
    .read_event_cb = read_event_handler,

    .uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {
            .uuid16 = &uuid_BATTERY_SERVICE_SVC,
        },
    },
};