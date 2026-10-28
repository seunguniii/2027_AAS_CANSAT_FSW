#pragma once

#include "Msg.hpp"

namespace Msg::detail{
  bool beginESPNOW();
  bool sendESPNOWPacket(const Packet& packet);
}
