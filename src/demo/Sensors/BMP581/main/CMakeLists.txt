idf_component_register(
    SRCS
        "app_main.cpp"
        "../sensors/barometer.cpp"
    INCLUDE_DIRS
        "../include"
    REQUIRES
        esp_wifi
        esp_netif
        esp_event
        nvs_flash
        
        esp_driver_i2c
        esp_driver_gpio
        esp_timer
)
