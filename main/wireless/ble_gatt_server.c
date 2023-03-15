
#include <string.h>

#include "ble_interface.h"
#include "ble_services.h"
#include "ble_gatt_server.h"
#include "ble_gap.h"

#include "esp_log.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"

#define TAG "BLE SERVER"
#define SVC_INST_ID 0
#define PREPARE_BUF_MAX_SIZE 1024

static uint16_t gatts_mtu = 23;

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
    uint16_t handle;
} prepare_type_env_t;

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

typedef struct
{
    uint16_t gatts_if;
    uint16_t conn_id;
    bool has_conn;
} gatts_profile_inst_t;

static gatts_profile_inst_t profile;
static prepare_type_env_t prepare_read_env;

static struct gatts_profile_inst bt_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};

// support read and long read
void gatts_proc_read(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_read_env, esp_ble_gatts_cb_param_t *param, uint8_t *p_rsp_v, uint16_t v_len)
{
    esp_err_t ret;

    if (!param->read.need_rsp)
    {
        return;
    }

    uint16_t value_len = gatts_mtu - 1;
    if (v_len - param->read.offset < (gatts_mtu - 1))
    { // read response will fit in one MTU?
        value_len = v_len - param->read.offset;
    }
    else if (param->read.offset == 0) // it's the start of a long read  (could also use param->read.is_long here?)
    {
        ESP_LOGI(TAG, "long read, handle = %d, value len = %d", param->read.handle, v_len);

        if (v_len > PREPARE_BUF_MAX_SIZE)
        {
            ESP_LOGE(TAG, "long read too long");
            return;
        }
        if (prepare_read_env->prepare_buf != NULL)
        {
            ESP_LOGW(TAG, "long read buffer not free");
            free(prepare_read_env->prepare_buf);
            prepare_read_env->prepare_buf = NULL;
        }

        prepare_read_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_read_env->prepare_len = 0;
        if (prepare_read_env->prepare_buf == NULL)
        {
            ESP_LOGE(TAG, "long read no mem");
            return;
        }
        memcpy(prepare_read_env->prepare_buf, p_rsp_v, v_len);
        prepare_read_env->prepare_len = v_len;
        prepare_read_env->handle = param->read.handle;
    }

    esp_gatt_rsp_t rsp;
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    rsp.attr_value.len = value_len;
    rsp.attr_value.offset = param->read.offset;
    memcpy(rsp.attr_value.value, &p_rsp_v[param->read.offset], value_len);

    ret = esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "send response error");
    }
}

// continuation of read, use buffered value
void gatts_proc_long_read(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_read_env, esp_ble_gatts_cb_param_t *param)
{

    if (prepare_read_env->prepare_buf && (prepare_read_env->handle == param->read.handle)) // something buffered?
    {
        gatts_proc_read(gatts_if, prepare_read_env, param, prepare_read_env->prepare_buf, prepare_read_env->prepare_len);
        if (prepare_read_env->prepare_len - param->read.offset < (gatts_mtu - 1)) // last read?
        {
            free(prepare_read_env->prepare_buf);
            prepare_read_env->prepare_buf = NULL;
            prepare_read_env->prepare_len = 0;
            ESP_LOGI(TAG, "long_read ended");
        }
    }
    else
    {
        ESP_LOGE(TAG, "long_read not buffered");
    }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    esp_err_t ret = ESP_OK;

    switch (event)
    {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);

        profile.gatts_if = gatts_if;

        ret = ble_gap_set_advertising_data();
        if (ret)
        {
            ESP_LOGE(TAG, "set adv data failed, error code = %x", ret);
        }

        ble_gatt_server_service_t *current = NULL;
        for (uint8_t i = 0; i < ble_gatt_server_services_len; i++)
        {
            current = ble_gatt_server_services[i];
            ret = esp_ble_gatts_create_attr_tab(current->gatt_db, gatts_if, current->gatt_db_len, i);
            if (ret)
            {
                ESP_LOGE(TAG, "create attr table failed, error code = %x", ret);
            }
        }

        break;
    case ESP_GATTS_READ_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_READ_EVT, conn_id %d, trans_id %ld, handle %d offset %d", param->read.conn_id, param->read.trans_id, param->read.handle, param->read.offset);

        if (!param->read.is_long)
        {
            // If is.long is false then this is the first (or only) request to read data
            int attrIndex;
            esp_gatt_rsp_t rsp;
            rsp.attr_value.len = 0;

            ble_gatt_server_service_t *current = NULL;
            for (uint8_t i = 0; i < ble_gatt_server_services_len; i++)
            {
                current = ble_gatt_server_services[i];
                if (!current->get_attribute || !current->read_event_cb)
                {
                    continue;
                }

                attrIndex = current->get_attribute(param->read.handle);
                if (attrIndex < current->gatt_db_len)
                {
                    ESP_LOGI(TAG, "Found attribute %d", attrIndex);

                    ret = current->read_event_cb(attrIndex, param, &rsp);

                    if (ret != ESP_OK)
                    {
                        ESP_LOGE(TAG, "read event error");
                    }
                }
            }

            // Helper function sends what it can (up to MTU size) and buffers the rest for later.
            gatts_proc_read(gatts_if, &prepare_read_env, param, rsp.attr_value.value, rsp.attr_value.len);
        }
        else // a continuation of a long read.
        {
            // Dont invoke the handle#SERVICE#ReadEvent again, just keep pumping out buffered data.
            gatts_proc_long_read(gatts_if, &prepare_read_env, param);
        }

        break;
    case ESP_GATTS_WRITE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT");
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        gatts_mtu = param->mtu.mtu;
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status %d", param->conf.status);
        break;
    case ESP_GATTS_UNREG_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_UNREG_EVT");
        break;
    case ESP_GATTS_CREATE_EVT:
        ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d", param->create.status, param->create.service_handle);
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        ESP_LOGI(TAG, "ADD_INCL_SERVICE_EVT, status %d,  service_handle %d", param->add_incl_srvc.status, param->add_incl_srvc.service_handle);
        break;
    case ESP_GATTS_ADD_CHAR_EVT:
        ESP_LOGI(TAG, "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d", param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
        break;
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        ESP_LOGI(TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d", param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
        break;
    case ESP_GATTS_DELETE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DELETE_EVT");
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:
        ESP_LOGI(TAG, "SERVICE_STOP_EVT, status %d, service_handle %d", param->stop.status, param->stop.service_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d", param->connect.conn_id);
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, param->connect.remote_bda, 6, ESP_LOG_INFO);

        gatts_mtu = 23;
        profile.conn_id = param->connect.conn_id;
        profile.has_conn = true;

        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.min_int = 0x10;
        conn_params.max_int = 0x20;
        conn_params.timeout = 400;

        // start sent the update connection parameters to the peer device.
        ret = esp_ble_gap_update_conn_params(&conn_params);
        if (ret)
        {
            ESP_LOGE(TAG, "update connection parameters error, error code = %x", ret);
        }
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, reason = %d", param->disconnect.reason);
        profile.has_conn = false;

        ret = ble_gap_start_advertising();
        if (ret)
        {
            ESP_LOGE(TAG, "start advertising failed, error code = %x", ret);
        }

        break;
    case ESP_GATTS_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_OPEN_EVT");
        break;
    case ESP_GATTS_CANCEL_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
        break;
    case ESP_GATTS_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CLOSE_EVT");
        break;
    case ESP_GATTS_LISTEN_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_LISTEN_EVT");
        break;
    case ESP_GATTS_CONGEST_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_CONGEST_EVT");
        break;
    case ESP_GATTS_RESPONSE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_RESPONSE_EVT");
        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        ESP_LOGI(TAG, "The number handle = %x", param->add_attr_tab.num_handle);

        ret = ESP_OK;

        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ESP_LOGE(TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
        }
        else
        {
            ble_gatt_server_service_t *current = NULL;
            for (uint8_t i = 0; i < ble_gatt_server_services_len; i++)
            {
                current = ble_gatt_server_services[i];

                if (param->add_attr_tab.svc_uuid.len != current->uuid.len)
                {
                    continue;
                }

                if (param->add_attr_tab.svc_uuid.uuid.uuid16 == *current->uuid.uuid.uuid16)
                {
                    if (param->add_attr_tab.num_handle != current->gatt_db_len)
                    {
                        ESP_LOGE(TAG, "create attribute table failed, num_handle not match handle = %d", current->gatt_db_len);
                        continue;
                    }
                    else
                    {
                        ESP_LOGI(TAG, "create attribute table successfully, the number handle = %d\n", param->add_attr_tab.num_handle);
                        memcpy(current->table_handle, param->add_attr_tab.handles, sizeof(*current->table_handle));
                        ret = esp_ble_gatts_start_service(current->table_handle[current->service_uuid_index]);
                        if (ret)
                        {
                            ESP_LOGE(TAG, "start service failed, error code = %x", ret);
                        }
                    }
                }
            }
        }

        break;
    case ESP_GATTS_SET_ATTR_VAL_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_SET_ATTR_VAL_EVT");
        break;
    case ESP_GATTS_SEND_SERVICE_CHANGE_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_SEND_SERVICE_CHANGE_EVT");
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
            bt_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGE(TAG, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == bt_profile_tab[idx].gatts_if)
            {
                if (bt_profile_tab[idx].gatts_cb)
                {
                    bt_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

esp_err_t ble_gatt_server_init(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initializing Bluetooth Interface");

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(TAG, "Gatts register failed with error code: %x", ret);
        return ret;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret)
    {
        ESP_LOGE(TAG, "App register failed with error code: %x", ret);
        return ret;
    }

    ret = esp_ble_gatt_set_local_mtu(517);
    if (ret)
    {
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", ret);
    }

    ESP_LOGI(TAG, "Bluetooth Interface initialized successfully");

    return ret;
}

esp_err_t ble_gatt_server_deinit(void)
{
    esp_err_t ret = ESP_OK;

    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED)
    {
        // esp_ble_gatts_app_unregister(//gattsif);
    }

    return ret;
}