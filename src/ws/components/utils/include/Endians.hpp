#pragma once

#include <cstdint>
#include <cstddef>

namespace utils{
  //little endians
  uint16_t readUInt16LE(const uint8_t* data) {
    return static_cast<uint16_t>(data[0])
           | (static_cast<uint16_t>(data[1]) << 8);
  }

  uint32_t readUInt32LE(const uint8_t* data) {
    return static_cast<uint32_t>(data[0])
           | (static_cast<uint32_t>(data[1]) << 8)
           | (static_cast<uint32_t>(data[2]) << 16)
           | (static_cast<uint32_t>(data[3]) << 24);
  }
  
  uint16_t readUInt16BE(const uint8_t* data) {
    return (static_cast<uint16_t>(data[0]) << 8)
           | static_cast<uint16_t>(data[1]);
  }
  
  
  //big endians
  uint32_t readUInt24BE(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 16)
           | (static_cast<uint32_t>(data[1]) << 8)
           | static_cast<uint32_t>(data[2]);
  }

  uint32_t readUInt32BE(const uint8_t* data) {
    return (static_cast<uint32_t>(data[0]) << 24)
           | (static_cast<uint32_t>(data[1]) << 16)
           | (static_cast<uint32_t>(data[2]) << 8)
           | static_cast<uint32_t>(data[3]);
  }
}
