idf_component_register(
  SRCS
    "Msg.cpp"
    "MsgESPNOW.cpp"
  INCLUDE_DIRS
    "include"
  REQUIRES
    cmd
    sensors
    system
    telemetry
    test
    
    esp_wifi
    esp_netif
    esp_event
    nvs_flash
)
