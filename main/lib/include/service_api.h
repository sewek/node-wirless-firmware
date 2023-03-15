
#pragma once

#include "services_conf.h"

uint8_t get_gatts_services_len();
esp_err_t update_service_config(gatts_service_t *);
esp_err_t init_services();
