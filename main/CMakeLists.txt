
set(srcs    "main.c")
set(include_dirs    ".")

if(CONFIG_BT_ENABLED)
    list(APPEND srcs "wireless/ble_gap.c"
                     "wireless/ble_gatt_server.c"
                     "wireless/ble_interface.c")

    list(APPEND include_dirs "wireless")
    list(APPEND include_dirs "wireless/services")

    if(CONFIG_BT_DEVICE_INFO_SERVICE_ENABLE)
        list(APPEND srcs "wireless/services/device_info_service.c")
    endif()

    if(CONFIG_BT_BATTERY_SERVICE_ENABLE)
        list(APPEND srcs "wireless/services/battery_service.c")
    endif()

endif()


idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}")