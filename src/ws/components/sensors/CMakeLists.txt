idf_component_register(
  SRCS
    "BMP581.cpp"
    "gnss.cpp"
    "imu.cpp"
    "magnetometer.cpp"
    "power_monitor.cpp"
  INCLUDE_DIRS
    "include"
  REQUIRES
    system
    interface
    utils
    
    esp_timer
)
