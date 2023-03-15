#include "device_info_service.h"

#include "esp_gatts_api.h"
#include "esp_log.h"

#define TAG "BT DEVICE INFO SERVICE"

uint16_t device_info_service_handle_table[DEVICE_INFO_SERVICE_NUM_ATTR];

const uint16_t uuid_DEVICE_INFO_SERVICE_SVC = ESP_GATT_UUID_DEVICE_INFO_SVC;
const uint16_t uuid_SYSTEM_ID = ESP_GATT_UUID_SYSTEM_ID;
const uint16_t uuid_MODEL_NUMBER_STR = ESP_GATT_UUID_MODEL_NUMBER_STR;
const uint16_t uuid_SERIAL_NUMBER_STR = ESP_GATT_UUID_SERIAL_NUMBER_STR;
const uint16_t uuid_FIRMWARE_VERSION_STR = ESP_GATT_UUID_FW_VERSION_STR;
const uint16_t uuid_HARDWARE_VERSION_STR = ESP_GATT_UUID_HW_VERSION_STR;
const uint16_t uuid_SOFTWARE_VERSION_STR = ESP_GATT_UUID_SW_VERSION_STR;
const uint16_t uuid_MANUFACTURER_NAME = ESP_GATT_UUID_MANU_NAME;
const uint16_t uuid_IEEE_DATA = ESP_GATT_UUID_IEEE_DATA;
const uint16_t uuid_PNP_ID = ESP_GATT_UUID_PNP_ID;

// System ID characteristic
#define DEVICE_INFO_SERVICE_SYSTEM_ID_LEN (8)
static uint8_t devInfoSystemId[DEVICE_INFO_SERVICE_SYSTEM_ID_LEN] = {0, 0, 0, 0, 0, 0, 0, 0};

#define DEVICE_INFO_SERVICE_STR_ATTR_LEN (20)

// Model Number String characteristic
static uint8_t devInfoModelNumber[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_MODEL_NUMBER;

// Serial Number String characteristic
static uint8_t devInfoSerialNumber[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_SERIAL_NUMBER;

// Firmware Revision String characteristic
static uint8_t devInfoFirmwareRev[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_FIRMWARE_REVISION;

// Hardware Revision String characteristic
static uint8_t devInfoHardwareRev[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_HARDWARE_REVISION;

// Software Revision String characteristic
static uint8_t devInfoSoftwareRev[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_SOFTWARE_REVISION;

// Manufacturer Name String characteristic
static uint8_t devInfoManufacturerName[DEVICE_INFO_SERVICE_STR_ATTR_LEN + 1] = CONFIG_BT_DEVICE_INFO_SERVICE_MANUFACTURER_NAME;

// IEEE 11073-20601 Regulatory Certification Data List characteristic
// IEEE 11073 authoritative body values
#define DEVICE_INFO_SERVICE_11073_BODY_EMPTY 0
#define DEVICE_INFO_SERVICE_11073_BODY_IEEE 1
#define DEVICE_INFO_SERVICE_11073_BODY_CONTINUA 2
#define DEVICE_INFO_SERVICE_11073_BODY_EXP 254
static uint8_t defaultDevInfo11073Cert[] =
    {
        DEVICE_INFO_SERVICE_11073_BODY_EXP, // authoritative body type
        0x00,                               // authoritative body structure type
                                            // authoritative body data follows below:
        'e', 'x', 'p', 'e', 'r', 'i', 'm', 'e', 'n', 't', 'a', 'l'};

// The length of this characteristic is not fixed
static uint8_t *devInfo11073Cert = defaultDevInfo11073Cert;
static uint8_t devInfo11073CertLen = sizeof(defaultDevInfo11073Cert);

// PnP ID characteristic
#define DEVICE_INFO_SERVICE_PNP_ID_LEN (7)
static uint8_t devInfoPnpId[DEVICE_INFO_SERVICE_PNP_ID_LEN] =
    {
        1,          // Vendor ID source (1=Bluetooth SIG)
        0xE5, 0x02, // Vendor ID (See https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers )
        0x00, 0x00, // Product ID (vendor-specific)
        0x10, 0x01  // Product version (JJ.M.N)
};

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
// static const uint16_t character_description_uuid = ESP_GATT_UUID_CHAR_DESCRIPTION;
// static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
// static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

const esp_gatts_attr_db_t device_info_service_gatt_db[DEVICE_INFO_SERVICE_NUM_ATTR] = {
    [DEVICE_INFO_SERVICE_SVC] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uuid_DEVICE_INFO_SERVICE_SVC), (uint8_t *)&uuid_DEVICE_INFO_SERVICE_SVC}},

    [DEVICE_INFO_SERVICE_SYSTEM_ID_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_SYSTEM_ID_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SYSTEM_ID, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoSystemId), (uint8_t *)&devInfoSystemId}},

    [DEVICE_INFO_SERVICE_MODEL_NUMBER_STR_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_MODEL_NUMBER_STR_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_MODEL_NUMBER_STR, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoModelNumber), (uint8_t *)&devInfoModelNumber}},

    [DEVICE_INFO_SERVICE_SERIAL_NUMBER_STR_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_SERIAL_NUMBER_STR_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SERIAL_NUMBER_STR, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoSerialNumber), (uint8_t *)&devInfoSerialNumber}},

    [DEVICE_INFO_SERVICE_FIRMWARE_VERSION_STR_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_FIRMWARE_VERSION_STR_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_FIRMWARE_VERSION_STR, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoFirmwareRev), (uint8_t *)&devInfoFirmwareRev}},

    [DEVICE_INFO_SERVICE_HARDWARE_VERSION_STR_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_HARDWARE_VERSION_STR_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_HARDWARE_VERSION_STR, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoHardwareRev), (uint8_t *)&devInfoHardwareRev}},

    [DEVICE_INFO_SERVICE_SOFTWARE_VERSION_STR_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_SOFTWARE_VERSION_STR_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_SOFTWARE_VERSION_STR, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoSoftwareRev), (uint8_t *)&devInfoSoftwareRev}},

    [DEVICE_INFO_SERVICE_MANUFACTURER_NAME_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_MANUFACTURER_NAME_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_MANUFACTURER_NAME, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoManufacturerName), (uint8_t *)&devInfoManufacturerName}},

    [DEVICE_INFO_SERVICE_IEEE_DATA_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_IEEE_DATA_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_IEEE_DATA, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(defaultDevInfo11073Cert), (uint8_t *)&defaultDevInfo11073Cert}},

    [DEVICE_INFO_SERVICE_PNP_ID_CHAR] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, sizeof(uint8_t), sizeof(char_prop_read), (uint8_t *)&char_prop_read}},
    [DEVICE_INFO_SERVICE_PNP_ID_VAL] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&uuid_PNP_ID, ESP_GATT_PERM_READ, ESP_GATT_MAX_ATTR_LEN, sizeof(devInfoPnpId), (uint8_t *)&devInfoPnpId}},
};

const ble_gatt_server_service_t device_info_service = {
    .service_uuid_index = DEVICE_INFO_SERVICE_SVC,
    .table_handle = &device_info_service_handle_table,
    .gatt_db = &device_info_service_gatt_db,
    .gatt_db_len = DEVICE_INFO_SERVICE_NUM_ATTR,
    .uuid = {
        .len = ESP_UUID_LEN_16,
        .uuid = {
            .uuid16 = &uuid_DEVICE_INFO_SERVICE_SVC,
        },
    },
};