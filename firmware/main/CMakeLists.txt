idf_component_register(SRCS "main.c" "board.c" "display.c" "imu.c" "quota.c" "quota_network.c" "ambience.c" "interaction.c" "touch_input.c" "dashboard.c"
 INCLUDE_DIRS "." REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_spi esp_lcd esp_timer efuse esp_wifi esp_event esp_netif esp_http_client esp_http_server nvs_flash esp_partition cjson)
target_compile_options(${COMPONENT_LIB} PRIVATE -O2)
