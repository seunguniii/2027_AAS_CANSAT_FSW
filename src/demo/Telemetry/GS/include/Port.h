#pragma once

#include<cstdint>
#include "Msg.h"

namespace Port{
  enum class FrameType: uint8_t{
    CTR_TELEMETRY = 0,
    PQ_TELEMETRY = 1,
    
    CMD = 2,
  };
  
  bool begin();
  bool sendPacket(FrameType type, const Msg::Packet& packet);
  bool getPacket(FrameType& type, Msg::Packet& packet);
}
