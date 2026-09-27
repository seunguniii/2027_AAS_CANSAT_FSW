#include<mutex>

#include "Msg.h"
#include "MsgESPNOW.h"

namespace Msg::detail{  
  constexpr size_t QUEUE_SIZE = 128;
  
  static Packet queue[QUEUE_SIZE];
  static size_t queue_count = 0;  
  
  static std::mutex queue_mutex;
  
  bool enqueuePacket(const Packet& packet){
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    if(queue_count >= QUEUE_SIZE) return false;
    
    queue[queue_count] = packet;
    queue_count++;
    
    return true;
  }
  
  bool sendPacket(const Packet& packet){
    //local msg
    if(packet.header.sender.mcu == packet.header.receiver.mcu) return enqueuePacket(packet);
    
    //inter-mcu msg
    return sendESPNOWPacket(packet);
    
    //use below line instead of line30 for compile testing
    //return true;
  }
  
  //COMMON RECEIVER
  bool getPacket(Packet& packet, Address receiver, MsgType msg_type, uint16_t length){
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    for(size_t i = 0; i < queue_count; i++){
      //if not mine skip
      if(queue[i].header.receiver.mcu != receiver.mcu) continue;
      if(queue[i].header.receiver.node != receiver.node) continue;
      if(queue[i].header.msg_type != msg_type) continue;
      if(queue[i].header.length != length) continue;
    
      packet = queue[i];
      
      //packet consumed, decrease count
      for(size_t j = i; j + 1 < queue_count; j++){
        queue[j] = queue[j+1];
      }
      queue_count--;
      queue[queue_count] = Packet{};
      
      return true;
    }
    return false;
  }
  
  //overloader for getPacket, used in bridge.cpp of GS
  bool getPacket(Packet& packet, Address receiver){
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    for(size_t i = 0; i < queue_count; i++){
      if(queue[i].header.receiver.mcu != receiver.mcu) continue;
      if(queue[i].header.receiver.node != receiver.node) continue;
      
      packet = queue[i];
      
      for(size_t j = i; j + 1 < queue_count; j++){
        queue[j] = queue[j+1];
      }
      queue_count--;
      queue[queue_count] = Packet{};
      
      return true;
    }
    return false;
  }
}//namespace msg::detail
