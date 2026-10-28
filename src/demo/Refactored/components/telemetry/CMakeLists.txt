idf_component_register(
  SRCS
    "CTR.cpp"
    "PQ.cpp"
  INCLUDE_DIRS "include"
  REQUIRES
    system
    sensors
    cmd
    uitls
)
