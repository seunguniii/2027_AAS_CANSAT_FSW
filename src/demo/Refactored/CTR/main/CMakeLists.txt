idf_component_register(
  SRCS
    "app_main.cpp"
  REQUIRES
    msg
    sensors
    system
    telemetry
    
    esp_timer
)
