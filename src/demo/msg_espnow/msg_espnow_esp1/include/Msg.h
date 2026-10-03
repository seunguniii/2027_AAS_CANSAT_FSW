#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include "Nodes.h"
#include "MsgTypes.h"

namespace Msg{  
  //only the payload, not the whole packet
  constexpr uint16_t MAX_PAYLOAD_SIZE = 128;
  
  using Address = ::Node::Address;
  
  struct Header{
    Address sender;	//2bytes
    Address receiver;	//2bytes
    MsgType msg_type;	//2bytes
    uint16_t length;	//2bytes
  };			//8bytes
  struct Packet{
    Header header;
    uint8_t payload[MAX_PAYLOAD_SIZE];
  };
  
  //compiler level check
  static_assert(sizeof(Header) == 8); //header has 8 bytes
  static_assert(sizeof(Packet) == sizeof(Header) + MAX_PAYLOAD_SIZE);
  
  
  //inner functions
  namespace detail{  
    bool sendPacket(const Packet& packet);
    bool getPacket(Packet& packet, Address receiver);
    bool getPacket(Packet& packet, Address receiver, MsgType type, uint16_t length);
    bool enqueuePacket(const Packet& packet);
  }
  
  class Node{
    public:
      explicit Node(Address address): address(address) {}
      
      Address getAddress() const{
        return address;
      }
      
      template <typename T>
      bool send(const T& message, Address receiver) const {   
        static_assert(
          std::is_trivially_copyable_v<T>,
          "Message must be trivially copyable"
        );
    
        static_assert(
          sizeof(T) <= MAX_PAYLOAD_SIZE,
          "Message is too large"
        );
    
        Packet packet{};
    
        packet.header.sender = address;
        packet.header.receiver = receiver;
        packet.header.msg_type = IDof<T>::type;
        packet.header.length = sizeof(T);
    
        std::memcpy(packet.payload, &message, sizeof(T));
    
        return detail::sendPacket(packet);
      }
  
      template<typename T>
      bool get(T& message) const {
        static_assert(
          std::is_trivially_copyable_v<T>,
          "Message must be trivially copyable"
        );
        static_assert(
          sizeof(T) <= MAX_PAYLOAD_SIZE,
          "Messag is too large"
        );
        Packet packet;
    
        if(!detail::getPacket(packet, address, 
           IDof<T>::type, static_cast<uint16_t>(sizeof(T)))) return false;
           
        if(packet.header.length != sizeof(T)) return false;
    
        std::memcpy(&message, packet.payload, sizeof(T));
    
        return true;
      }
    //</public>
      
    private:
      Address address;
    //</private>
  };
}
