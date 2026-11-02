#pragma once

#include <cstdint>

namespace CMD {
    enum class CTR: uint8_t {
        INIT = 0
    };
    
    enum class PQ: uint8_t {
        INIT = 0,
        
        IMG_STB = 50,
        
        SCI_EXP = 100,
    };
}
