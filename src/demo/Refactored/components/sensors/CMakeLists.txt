idf_component_register(
  SRCS
    "BMP581.cpp"
    "power_monitor.cpp"
  INCLUDE_DIRS "include"
  REQUIRES
    system
    interfaces
)
