#pragma once

#include <cstdint>

namespace MCU{enum class ID: std::uint8_t;}

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
