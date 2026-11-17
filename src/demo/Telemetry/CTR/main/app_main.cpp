#include <iostream>
#include <cstdio>
#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Msg.h"
#include "MsgESPNOW.h"
#include "Sensors.h"
#include "Telemetry.h"

constexpr MCU::ID CTR_MCU = MCU::ID::ESP0;
constexpr MCU::ID GS_MCU = MCU::ID::ESP1;

const Msg::Node ctr_telemetry_node{Node::Address{CTR_MCU, Node::ID::TELEMETRY}};
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};


int main(void)
{
  std::cout << "[CTR] Initializing...\n";
  
  if(!Msg::detail::beginESPNOW()){
    std::cout << "[CTR] ESP-NOW initialization failed.\n";
    return 1;
  }
  
  uint32_t current_time = 0;  
  
  //telemetry
  uint32_t last_telemetry_time = 0;
  uint32_t TELEMETRY_PERIOD = 250; //[ms], depends on operation state, default 4Hz
  
  //sensors
  Sensor::Barometer latest_barometer{};
  Sensor::PowerMonitor latest_power_monitor{};
  
  uint32_t last_barometer_time = 0;
  uint32_t last_power_monitor_time = 0;
  constexpr uint32_t BAROMETER_PERIOD = 50; //[ms], 20Hz
  constexpr uint32_t POWER_MONITOR_PERIOD = 100;//[ms], 10Hz
  
  //states
  FlightMode mode = FlightMode::S;
  CTR_OpState state = CTR_OpState::LAUNCH_PAD;
  uint8_t mech_state = 0x00;
  
  //TODO Video::start_rec();
  
  std::cout << "[CTR] Initializing successful.\n";
  
  while(true){
    current_time = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    
    if(current_time - last_barometer_time >= BAROMETER_PERIOD){
      latest_barometer = Barometer::getData();
      last_barometer_time += BAROMETER_PERIOD;
    }
    
    if(current_time - last_power_monitor_time >= POWER_MONITOR_PERIOD){
      latest_power_monitor = PowerMonitor::getData();
      last_power_monitor_time += POWER_MONITOR_PERIOD;
    }
    
    //for checking telemetry cycle period
    if(current_time - last_telemetry_time >= TELEMETRY_PERIOD){                
      Telemetry::sendCTR(
        ctr_telemetry_node, gs_telemetry_node,
        latest_barometer, latest_power_monitor,
        mode, state, mech_state);
        
      last_telemetry_time += TELEMETRY_PERIOD;
    }
    
    //TODO
    //if(CMD_received == CMD::Video::stop) Video::stop_rec();
    
    //TODO
    //if(landed) return 0;      
    
    vTaskDelay(1);
  }
  return 1;
}

extern "C" void app_main()
{
    const int result = main();

    std::cout.flush();
    fflush(stdout);

    printf("\n[CTR] CTR returned %d\n", result);
}
