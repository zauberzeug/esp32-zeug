idf_component_register(
SRCS
    "include/esp_zeug/nvs-cache.h" "src/nvs-cache.cpp"
    "include/esp_zeug/httpd-util.h" "src/httpd-util.cpp"
    "include/esp_zeug/eventhandler.h"
    "include/esp_zeug/frtos-util.h"
    "include/esp_zeug/util.h"
    "include/esp_zeug/ble/uuid.h"
    "include/esp_zeug/ble/gatts.h"
INCLUDE_DIRS
    "include"
REQUIRES
    nvs_flash
    esp_http_server
    esp_http_client
)
component_compile_options(-std=gnu++17 -Wsuggest-override)
