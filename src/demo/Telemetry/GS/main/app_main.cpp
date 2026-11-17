#include <iostream>
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Msg.h"
#include "MsgESPNOW.h"
#include "Port.h"
#include "Bridge.h"
#include "Telemetry.h"

constexpr MCU::ID GS_MCU = MCU::ID::ESP1;
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};

int main(void)
{
  std::cout << "[GS] Initializing...\n";
  
  if(!Msg::detail::beginESPNOW()){
    std::cout << "[GS] ESP-NOW initialization failed.\n";
    return 1;
  }
  
  if(!Port::begin()){
    std::cout << "[GS] Port initialization failed.\n";
    return 1;
  }
  
  std::cout << "[GS] Initialization successful.\n";
  
  while(true){
    Bridge::forwardPacket(gs_telemetry_node);
    //TODO Bridge::forwardPacket(cmd?);
    
    vTaskDelay(1);
  }
  return 1;
}

extern "C" void app_main()
{
  const int result = main();
  
  std::cout.flush();
  fflush(stdout);
  
  printf("\n[GS] GS returned %d\n", result);
}
