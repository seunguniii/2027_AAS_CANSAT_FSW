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

}//namespace utils
