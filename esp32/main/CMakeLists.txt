idf_component_register(SRCS "opcua_esp32.c"
                    INCLUDE_DIRS "."
                    REQUIRES  driver esp_netif bootloader_support nvs_flash ethernet open62541lib model)
component_compile_options(-Wno-error=format= -Wno-format)