#pragma once

#include <cstdint>

namespace Test {
  struct __attribute__((__packed__)) MsgTest{
    int header;
    
    uint8_t test_uint8;
    float test_float;
  };
}
