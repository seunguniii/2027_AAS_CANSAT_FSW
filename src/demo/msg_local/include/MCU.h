#pragma once

#include <cstdint>

#include "Nodes.h"

namespace MCU{
  template <Node::ID... Nodes>
  
  class Device{
    public:
      constexpr explicit Device(ID id): id(id) {}
      constexpr ID getID() const {
        return id;
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
    };
    
    //SET NODE ADDRESS HERE
    //ADD NODE/MCU in Nodes.h
    inline constexpr auto PQ = 
      Device<
        //sensors
        Node::ID::IMU,
        Node::ID::BAROMETER,
        Node::ID::POWER_MONITOR,
        Node::ID::MAGNETOMETER,
        Node::ID::GNSS,
        
        //flight logic
        Node::ID::MAIN
      >(ID::PQ);
      
    inline constexpr auto CTR = 
      Device<
        //sensors
        Node::ID::IMU,
        Node::ID::BAROMETER,
        Node::ID::POWER_MONITOR,
        
        //flight logic
        Node::ID::MAIN
      >(ID::CTR);
      
    inline constexpr auto GND = 
      Device<>(ID::GND);
    
    
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
        
        Node::ID::MSG_ESPNOW_TEST
      >(ID::ESP0);
      
    inline constexpr auto ESP1 = 
      Device<
        Node::ID::MSG_LOCAL_NODE0_TEST,
        Node::ID::MSG_LOCAL_NODE1_TEST,
        
        Node::ID::MSG_ESPNOW_TEST
      >(ID::ESP1);
}
