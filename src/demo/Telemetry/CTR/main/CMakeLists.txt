idf_component_register(
  SRCS
    "app_main.cpp"
    "../msg/Msg.cpp"
    "../msg/MsgESPNOW.cpp"
    "../telemetry/CTR.cpp"
    "../sensors/barometer.cpp"
    "../sensors/power_monitor.cpp"
    
  INCLUDE_DIRS
    "../include"
  
  REQUIRES
    esp_timer
    esp_wifi
    esp_netif
    esp_event
    nvs_flash
    
    esp_driver_i2c
    esp_driver_gpio
)
