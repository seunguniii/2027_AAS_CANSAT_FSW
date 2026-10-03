#pragma once

#include <cstdint>

//SET ADDRESS IN MCU.h
namespace MCU{
  //ADD MCU HERE
  enum class ID: uint8_t{
    //real systems (0-199)
    PQ = 0,
    CTR = 1,
    GS = 2,
    
    
    //test/demo systems (200-255)
    TEST = 200, //machine test for code check
    
    ESP0 = 201,
    ESP1 = 202,
    ESP_CAM_0 = 203,
    ESP_CAM_1 = 204,
  };
}


namespace Node{
  //ADD NODES HERE
  enum class ID : uint8_t {
    //sensor drivers (50-99)
    IMU = 50,
    BAROMETER = 51,
    POWER_MONITOR = 52,
    MAGNETOMETER = 53,
    GNSS = 54,


    //flight logic (0-16)
    MAIN = 0,
    TELEMETRY = 1,
    
    
    //test (200-255)
    HEADER_TEST = 200,
    
    MSG_LOCAL_NODE0_TEST = 201,
    MSG_LOCAL_NODE1_TEST = 202,
    
    MSG_ESPNOW_TEST = 203,
  };
  
  struct Address {
    MCU::ID mcu;
    Node::ID node;
  };
  
  //check if all address are set correctly in compiling stage
  static_assert(sizeof(Address) == 2);
}
