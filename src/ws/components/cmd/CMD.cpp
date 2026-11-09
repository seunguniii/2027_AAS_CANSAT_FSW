#include "CMD.hpp"

#include <cstdint>

#include "Nodes.hpp"
#include "CRC.hpp"
#include "Endians.hpp"

namespace CMD{
  constexpr size_t CMD_LENGTH = sizeof(CMD); //bytes
  uint16_t packet_count = 1;
  
  CMD buildCMD(Node::Address rx_addr, uint16_t sq_num, 
               Verb verb, Noun noun, uint32_t param){
    CMD cmd{};
    
    cmd.rx_addr = rx_addr;
    cmd.sequence_number = sq_num;
    
    cmd.verb = verb;
    cmd.noun = noun;
    cmd.param = param;
    
    utils::buildCRC(cmd);
    
    return cmd;
  }
  
  
  bool sendCMD(const Msg::Node this_node, const Node::Address rx_addr, const uint16_t sq_num,
               const Verb verb, const Noun noun, const uint32_t param){
    CMD cmd = buildCMD(rx_addr, sq_num, verb, noun, param);
    packet_count++;
    
    return this_node.send(cmd, rx_addr);
  }
  
  
  RxResult getCMD(const Msg::Node& my_node, CMD& cmd){
    Msg::Packet packet{};
    
    const Node::Address my_addr = my_node.getAddress();
    
    if(!Msg::detail::getPacket(packet, my_addr, 
                               Msg::IDof<CMD>::type, sizeof(CMD)))
      return RxResult::NO_PACKET;
    
    CMD rx_cmd{};
    std::memcpy(&rx_cmd, packet.payload, sizeof(CMD));
    
    if(!utils::verifyCRC(rx_cmd)) return RxResult::INVALID_CRC;
    if(!validateCMD(rx_cmd)) return RxResult::INVALID_CMD;
    
    cmd = rx_cmd;
    return RxResult::ACCEPTED;
  }
  
  bool validateCMD(const CMD& cmd){
    switch(command.verb){
      case Verb::START:
      case Verb::STOP:
      case Verb::KILL:
        if(cmd.noun != Noun::TASK) return false;
        //if(!command.param.memberOf(CMD::TaskID)) return false; //TODO
        return true;
      
      case Verb::REBOOT:
        return cmd.noun == Noun.MCU && cmd.param == 0;
      
      case Verb::CALIBRATE:
        return cmd.noun == Noun::ALTITUDE
               || cmd.noun == Noun::IMU
               || cmd.noun == Noun::TIME;
      
      case Verb::ENABLE:
        return cmd.noun == Noun::FLIGHTMODE
               && (cmd.param == 0 || cmd.param == 1);
      
      case Verb::ACTIVATE:
        return cmd.noun == Noun::FLIGHTMODE
               || Noun::MECHANISM;
      
      default:
        return false;
  }
}
