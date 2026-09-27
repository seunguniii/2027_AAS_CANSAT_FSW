#include<mutex>

#include "Msg.h"

namespace Msg::detail{  
  constexpr size_t QUEUE_SIZE = 128;
  
  static Packet queue[QUEUE_SIZE];
  static size_t queue_count = 0;  
  
  static std::mutex queue_mutex;
  
  //local msg
  bool sendPacket(const Packet& packet){
    //if different mcu fallback to espnow
    if(packet.header.sender.mcu != packet.header.receiver.mcu) return false;
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    //if exceeded wanted size don't send
    if(queue_count >= QUEUE_SIZE) return false;
    
    queue[queue_count] = packet;
    queue_count++;
  
    return true;
  }
  
  bool getPacket(Packet& packet, Address receiver, MsgType msg_type, uint16_t length){
    std::lock_guard<std::mutex> lock(queue_mutex);
    
    for(size_t i = 0; i < queue_count; i++){
      //if not mine skip
      if(queue[i].header.receiver.mcu != receiver.mcu) continue;
      if(queue[i].header.receiver.node != receiver.node) continue;
      if(queue[i].header.msg_type != msg_type) continue;
      if(queue[i].header.length != length) continue;
    
      packet = queue[i];
      
      for(size_t j = i; j + 1 < queue_count; j++){
        queue[j] = queue[j+1];
      }
      queue_count--; //packet consumed, decrease count
      queue[queue_count] = Packet{};
      
      return true;
    }
    return false;
  }
}
