idf_component_register(
  SRCS
    "app_main.cpp"
    "../bridge/Bridge.cpp"
    "../msg/Msg.cpp"
    "../msg/MsgESPNOW.cpp"
    "../port/Port.cpp"
    
  INCLUDE_DIRS
    "../include"
  
  REQUIRES
    esp_wifi
    esp_netif
    esp_event
    nvs_flash
    esp_driver_usb_serial_jtag
)
