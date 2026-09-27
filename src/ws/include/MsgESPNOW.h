#pragma once

#include "Msg.h"

namespace Msg::detail{
  bool beginESPNOW();
  bool sendESPNOWPacket(const Packet& packet);
}
