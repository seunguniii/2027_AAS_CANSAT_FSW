idf_component_register(
  SRCS
    "Msg.cpp"
    "MsgESPNOW.cpp"
  INCLUDE_DIRS "include"
  REQUIRES
    system
    sensors
    cmd
    test
    telemetry
)
