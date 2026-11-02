#include <iostream>
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MCU.hpp"
#include "Msg.hpp"
#include "MsgESPNOW.hpp"
#include "Port.hpp"
#include "Bridge.hpp"

constexpr MCU::ID GS_MCU = MCU::ID::ESP_CAM_0;
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};

extern "C" void app_main()
{
  std::cout << "[GS] Initializing...\n";
  
  if(!Msg::detail::beginESPNOW()) std::cout << "[GS] ESP-NOW initialization failed.\n";
  
  if(!Port::begin()) std::cout << "[GS] Port initialization failed.\n";
  
  std::cout << "[GS] Initialization successful.\n";
  
  while(true){
    Bridge::forwardPacket(gs_telemetry_node);
    //TODO Bridge::forwardPacket(cmd?);
    
    vTaskDelay(1);
  }
}
