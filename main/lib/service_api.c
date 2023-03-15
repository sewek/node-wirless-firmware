#include "esp_log.h"
#include "esp_err.h"

#include "service_api.h"
#include "service_types.h"

#include "sdkconfig.h"

#define SERVICE_API_LOG_TAG "SERVICE_API"
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME

#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    // second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

// The length of adv data must be less than 31 bytes
// static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
// adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    //.min_interval = 0x0006,
    //.max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

gatts_service_t *gatts_services[] = {

#ifdef CONFIG_BT_BATTERY_SERVICE_ENABLE
#include "battery_service.h"
    &battery_service_conf,
#endif

    NULL,
};

static uint8_t adv_config_done = 0;

uint8_t get_gatts_services_len()
{
    uint8_t count = 0;

    while (gatts_services[count] != NULL)
    {
        count++;
    }

    return count;
}

esp_err_t update_service_config(gatts_service_t *service)
{
    return ESP_OK;
    /* for (uint16_t i = 0; i < get_gatts_services_len(); i++)
    {
        if (gatts_services[i].app_id == service->app_id)
        {
            gatts_services[i].gatts_if = service->gatts_if;
            gatts_services[i].conn_id = service->conn_id;
            gatts_services[i].char_handle = service->char_handle;
            gatts_services[i].char_uuid = service->char_uuid;
            gatts_services[i].perm = service->perm;
            gatts_services[i].property = service->property;
            gatts_services[i].descr_uuid = service->descr_uuid;

            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND; */
}

gatts_service_t *get_service_with_id(uint16_t app_id)
{
    ESP_LOGI(SERVICE_API_LOG_TAG, "get_service_with_id: %d", app_id);
    for (uint16_t i = 0; i < get_gatts_services_len(); i++)
    {
        if (gatts_services[i]->app_id == app_id)
        {
            ESP_LOGI(SERVICE_API_LOG_TAG, "returning servie: %d", app_id);
            return gatts_services[i];
        }
        else
        {
            ESP_LOGI(SERVICE_API_LOG_TAG, "not returning servie: %d", app_id);
        }
    }

    return NULL;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {

    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(SERVICE_API_LOG_TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(SERVICE_API_LOG_TAG, "Advertising stop failed\n");
        }
        else
        {
            ESP_LOGI(SERVICE_API_LOG_TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(SERVICE_API_LOG_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gatts_service_t *service;
            service = get_service_with_id(param->reg.app_id);

            if (service == NULL)
            {
                ESP_LOGE(SERVICE_API_LOG_TAG, "Service not found for app_id %04x", param->reg.app_id);
                return;
            }

            service->gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGI(SERVICE_API_LOG_TAG, "Reg app failed, app_id %04x, status %d\n",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < get_gatts_services_len(); idx++)
        {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gatts_if == gatts_services[idx]->gatts_if)
            {
                if (gatts_services[idx]->gatts_cb)
                {
                    gatts_services[idx]->gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

esp_err_t init_services()
{
    esp_err_t ret;

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(SERVICE_API_LOG_TAG, "gatts register error, error code = %x", ret);
        return ret;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(SERVICE_API_LOG_TAG, "gap register error, error code = %x", ret);
        return ret;
    }

    for (uint8_t i = 0; i < get_gatts_services_len(); i++)
    {
        ret = esp_ble_gatts_app_register(gatts_services[i]->app_id);
        if (ret)
        {
            ESP_LOGE(SERVICE_API_LOG_TAG, "gatts app register error, error code = %x", ret);
            return ret;
        }
    }

    ret = esp_ble_gap_set_device_name(DEVICE_NAME);
    if (ret)
    {
        ESP_LOGE(SERVICE_API_LOG_TAG, "set device name failed, error code = %x", ret);
    }

    // config adv data
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret)
    {
        ESP_LOGE(SERVICE_API_LOG_TAG, "config adv data failed, error code = %x", ret);
    }
    adv_config_done |= adv_config_flag;

    // config scan response data
    ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
    if (ret)
    {
        ESP_LOGE(SERVICE_API_LOG_TAG, "config scan response data failed, error code = %x", ret);
    }
    adv_config_done |= scan_rsp_config_flag;

    return ESP_OK;
}