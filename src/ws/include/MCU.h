#pragma once

#include <array>
#include <cstdint>

#include "Nodes.h"

namespace MCU{
  using MacAddress = std::array<uint8_t, 6>;
  
  template <Node::ID... Nodes>
  
  class Device{
    public:
      constexpr explicit Device(ID id): id(id), mac_addr{} {}
      constexpr Device(ID id, MacAddress mac): id(id), mac_addr(mac) {}
      
      constexpr ID getID() const {
        return id;
      }
      constexpr const MacAddress& macAddress() const{
        return mac_addr;
      }
      
      constexpr bool contains(Node::ID node) const {
        return ((node == Nodes) || ...);
      }
      
      template <Node::ID NodeID>
      constexpr Node::Address address() const {
        static_assert(
          ((NodeID == Nodes) || ...),
          "Requested node does not belong to this MCU"
        );
        
        return {id, NodeID};
      }
      
    private:
      ID id;
      MacAddress mac_addr;
  };
    
    
  //SET NODE ADDRESS HERE
  //ADD NODE/MCU in Nodes.h
    
  //REAL SYSTEMS
  inline constexpr auto PQ = 
    Device<
      //sensors
      Node::ID::IMU,
      Node::ID::BAROMETER,
      Node::ID::POWER_MONITOR,
      Node::ID::MAGNETOMETER,
      Node::ID::GNSS,
      
      Node::ID::TELEMETRY,
      
      //flight logic
      Node::ID::MAIN
    >(ID::PQ);
    
  inline constexpr auto CTR = 
    Device<
      //sensors
      Node::ID::IMU,
      Node::ID::BAROMETER,
      Node::ID::POWER_MONITOR,
      
      Node::ID::TELEMETRY,
      
      //flight logic
      Node::ID::MAIN
    >(ID::CTR);
    
  inline constexpr auto GS = 
    Device<
      Node::ID::TELEMETRY
      >(ID::GS);
  
  
  //TEST  
  inline constexpr auto TEST = 
    Device<
      Node::ID::HEADER_TEST,
      
      Node::ID::MSG_LOCAL_NODE0_TEST,
      Node::ID::MSG_LOCAL_NODE1_TEST
    >(ID::TEST);
  
  
  //DEMO
  inline constexpr auto ESP0 = 
    Device<
      Node::ID::MSG_LOCAL_NODE0_TEST,
      Node::ID::MSG_LOCAL_NODE1_TEST,
      
      Node::ID::MSG_ESPNOW_TEST,
      
      Node::ID::TELEMETRY
    >(ID::ESP0, {0x14, 0xC1, 0x9F, 0x20, 0x06, 0x80});
     
  inline constexpr auto ESP1 = 
    Device<
      Node::ID::MSG_LOCAL_NODE0_TEST,
      Node::ID::MSG_LOCAL_NODE1_TEST,
      
      Node::ID::MSG_ESPNOW_TEST,
      
      Node::ID::TELEMETRY
    >(ID::ESP1, {0x44, 0x1B, 0xF6, 0xFD, 0xB6, 0x04});
    
  
  //mac lookup
  inline const MacAddress& macAddress(ID id){
    switch(id){
      case ID::PQ:
        return PQ.macAddress();
      case ID::CTR:
        return CTR.macAddress();
      case ID::GS:
        return GS.macAddress();
        
      case ID::TEST:
        return TEST.macAddress();
        
      case ID::ESP0:
        return ESP0.macAddress();
      case ID::ESP1:
        return ESP1.macAddress();
        
      default:
        break;
    }
    //invalid mac
    static constexpr MacAddress invalid{};
    return invalid;        
  }
}
