#include "ble_gap.h"

#include <inttypes.h>

#include "esp_log.h"
#include "esp_gap_ble_api.h"

#define TAG "BLE GAP"

#define ADV_INT_MIN 0x20
#define ADV_INT_MAX 0x40

static uint8_t adv_config_done = 0;

#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
    /* flags */
    0x02, 0x01, 0x06,
    /* tx power*/
    0x02, 0x0a, 0xeb,
    /* service uuid */
    0x03, 0x03, 0xab, 0xcd,
    /* device name */
    /* 0x0f, 0x09, 'N', 'o', 'd', 'e', ' ', 'S', 'e', 'n', 's', 'o', 'r' */
};
static uint8_t raw_scan_rsp_data[] = {
    /* flags */
    0x02,
    0x01,
    0x06,

    /* tx power */
    0x02,
    0x0a,
    0xeb,

    /* service uuid */
    0x03,
    0x03,
    0xFF,
    0x00,
};
#else

static uint8_t adv_service_uuid128[16] = {0xC0, 0x2D, 0x0D, 0xAE, 0x33, 0x57, 0x46, 0x0B, 0xB0, 0x37, 0xA2, 0x04, 0x5D, 0xE4, 0x3D, 0x82};

// The length of adv data must be less than 31 bytes
// static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
// adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = ADV_INT_MIN,
    .max_interval = ADV_INT_MAX,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
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
    .min_interval = ADV_INT_MIN,
    .max_interval = ADV_INT_MAX,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = ADV_INT_MIN,
    .adv_int_max = ADV_INT_MAX,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#else
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            ESP_LOGI(TAG, "ADV config done\n");
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#endif
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising start failed\n");
        }
        else
        {
            ESP_LOGI(TAG, "Advertising start successfully\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(TAG, "Advertising stop failed\n");
        }
        else
        {
            ESP_LOGI(TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
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

esp_err_t ble_gap_init(void)
{
    esp_err_t ret = ESP_OK;

    // register the gap callback function
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(TAG, "gap register error, error code = %x", ret);
        return ret;
    }

    /* ret = ble_gap_set_advertising_data();
    if (ret)
    {
        ESP_LOGE(TAG, "set adv data failed, error code = %x", ret);
        return ret;
    }

    ret = ble_gap_start_advertising();
    if (ret)
    {
        ESP_LOGE(TAG, "start adv failed, error code = %x", ret);
        return ret;
    } */

    return ret;
}

esp_err_t ble_gap_deinit(void)
{
    esp_err_t ret = ESP_OK;

    ret = ble_gap_stop_advertising();
    if (ret)
    {
        ESP_LOGE(TAG, "stop adv failed, error code = %x", ret);
        return ret;
    }

    return ret;
}

esp_err_t ble_gap_set_advertising_data(void)
{
    esp_err_t ret = ESP_OK;

    ret = esp_ble_gap_set_device_name(DEVICE_NAME);
    if (ret)
    {
        ESP_LOGE(TAG, "set device name failed, error code = %x", ret);
        return ret;
    }

#ifdef CONFIG_SET_RAW_ADV_DATA
    ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
    if (ret)
    {
        ESP_LOGE(TAG, "config raw adv data failed, error code = %x", ret);
    }
    adv_config_done |= ADV_CONFIG_FLAG;

    ret = esp_ble_gap_config_adv_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
    if (ret)
    {
        ESP_LOGE(TAG, "config raw scan response data failed, error code = %x", ret);
    }
    adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#else
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret)
    {
        ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
    }
    adv_config_done |= ADV_CONFIG_FLAG;

    ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
    if (ret)
    {
        ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
    }
    adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#endif

    return ret;
}

esp_err_t ble_gap_start_advertising(void)
{
    esp_err_t ret = ESP_OK;

    ret = esp_ble_gap_start_advertising(&adv_params);
    if (ret)
    {
        ESP_LOGE(TAG, "start adv failed, error code = %x", ret);
        return ret;
    }

    return ret;
}

esp_err_t ble_gap_stop_advertising(void)
{
    esp_err_t ret = ESP_OK;

    ret = esp_ble_gap_stop_advertising();
    if (ret)
    {
        ESP_LOGE(TAG, "stop adv failed, error code = %x", ret);
        return ret;
    }

    return ret;
}