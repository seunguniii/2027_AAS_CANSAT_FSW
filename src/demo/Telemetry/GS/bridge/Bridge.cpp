#include <iostream>

#include "Bridge.h"
#include "Port.h"

namespace Bridge{
  bool forwardPacket(const Msg::Node& this_node){
    Msg::Packet packet{};
    
    if(!Msg::detail::getPacket(packet, this_node.getAddress())) return false;
    
    Port::FrameType type;
    
    switch(packet.header.msg_type){
      case Msg::MsgType::CTR_TELEMETRY:
        type = Port::FrameType::CTR_TELEMETRY;
        break;
        
      case Msg::MsgType::PQ_TELEMETRY:
        type = Port::FrameType::PQ_TELEMETRY;
        break;
        
      default:
        return false;
    }
    
    return Port::sendPacket(type, packet);
  }
}

namespace CMD{
  //TODO bool forwardPacket() //GS -> CanSat
}
