#pragma once

#include <cstdint>
#include <cstddef>

namespace utils{
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
}
