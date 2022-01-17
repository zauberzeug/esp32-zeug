idf_component_register(
    SRCS
        "include/zauber/nvs-cache.h" "src/nvs-cache.cpp"
        "include/zauber/httpd-util.h" "src/httpd-util.cpp"
        "include/zauber/eventhandler.h"
        "include/zauber/frtos-util.h"
        "include/zauber/util.h"
    INCLUDE_DIRS
        "include"
    REQUIRES
        nvs_flash
        esp_http_server
        esp_http_client
)
component_compile_options(-std=gnu++17 -Wsuggest-override)
