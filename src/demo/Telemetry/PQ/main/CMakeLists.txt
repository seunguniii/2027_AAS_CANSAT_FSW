idf_component_register(
  SRCS
    "app_main.cpp"
    "../msg/Msg.cpp"
    "../msg/MsgESPNOW.cpp"
    "../telemetry/PQ.cpp"
    "../sensors/barometer.cpp"
    "../sensors/power_monitor.cpp"
    "../sensors/imu.cpp"
    "../sensors/magnetometer.cpp"
    "../sensors/gnss.cpp"
    
  INCLUDE_DIRS
    "../include"
  
  REQUIRES
    esp_timer
    esp_wifi
    esp_netif
    esp_event
    nvs_flash
)
