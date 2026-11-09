#pragma once

#include <cstdint>
#include <cstddef>

namespace utils{
  inline uint16_t calculateCRC16(const uint8_t* data, size_t length){
    uint16_t crc = 0xFFFF;
    for(size_t i = 0; i < length; ++i){
      crc ^= static_cast<uint16_t>(data[i]) << 8;
      for(uint8_t bit = 0; bit < 8; ++bit){
        if(crc & 0x8000) crc = (crc << 1) ^ 0x1021; //std crc16 polynomial
        else crc <<= 1;
      }
    }
    return crc;
  }//calculate crc()
  
  
  //helpers for crc (packet verification)  
  template<typename T>
  inline void buildCRC(T& packet){
    packet.crc = calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
  }
  
  template<typename T>
  inline bool verifyCRC(const T& packet){
    uint16_t computed = calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
    return (packet.crc == computed);
  }
}//namespace utils
