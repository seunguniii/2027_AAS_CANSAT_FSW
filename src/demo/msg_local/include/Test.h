#pragma once

#include <cstdint>

namespace Test {
  struct __attribute__((__packed__)) MsgTest{
    uint8_t test_uint8;
    float test_float;
  };
  
  static uint8_t answer_uint8 = 42;
  static float answer_float = 3.1415926535;
}
