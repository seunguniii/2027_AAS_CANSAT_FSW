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

constexpr MCU::ID PQ_MCU = MCU::ID::ESP0;
constexpr MCU::ID GS_MCU = MCU::ID::ESP1;

const Msg::Node pq_telemetry_node{Node::Address{PQ_MCU, Node::ID::TELEMETRY}};
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};


int main(void)
{
  std::cout << "[PQ] Initializing...\n";
  
  if(!Msg::detail::beginESPNOW()){
    std::cout << "[PQ] ESP-NOW initialization failed.\n";
    return 1;
  }
  
  uint32_t current_time = 0;  
  
  //telemetry
  uint32_t last_telemetry_time = 0;
  constexpr uint32_t TELEMETRY_PERIOD = 100; //[ms]
  
  //sensors
  Sensor::Barometer latest_barometer{};
  Sensor::PowerMonitor latest_power_monitor{};
  Sensor::IMU latest_imu{};
  Sensor::Magnetometer latest_magnetometer{};
  Sensor::GNSS latest_gnss{};
  Sensor::PowerMonitor latest_sp1_monitor{};
  Sensor::PowerMonitor latest_sp2_monitor{};
  
  uint32_t last_barometer_time = 0;
  uint32_t last_power_monitor_time = 0;
  uint32_t last_imu_time = 0;
  uint32_t last_magnetometer_time = 0;
  uint32_t last_gnss_time = 0;
  uint32_t last_sp_monitor_time = 0;
  
  constexpr uint32_t BAROMETER_PERIOD = 50; //[ms], 20Hz
  constexpr uint32_t POWER_MONITOR_PERIOD = 100;//[ms], 10Hz
  constexpr uint32_t IMU_PERIOD = 10; //[ms], 4Hz
  constexpr uint32_t MAGNETOMETER_PERIOD = 100;//[ms], 10Hz
  constexpr uint32_t GNSS_PERIOD = 50;//[ms], 20Hz
  constexpr uint32_t SP_MONITOR_PERIOD = 100;//[ms], 10Hz
  
  
  //states
  FlightMode mode = FlightMode::SIMULATION;
  uint8_t mech_state = 0x0000;
  
  //TODO Video::start_rec();
  
  std::cout << "[PQ] Initialization successful.\n";
  
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
    
    if(current_time - last_imu_time >= IMU_PERIOD){
      latest_imu = IMU::getData();
      last_imu_time += IMU_PERIOD;
    }
    
    if(current_time - last_magnetometer_time >= MAGNETOMETER_PERIOD){
      latest_magnetometer = Magnetometer::getData();
      last_magnetometer_time += MAGNETOMETER_PERIOD;
    }
    
    
    if(current_time - last_gnss_time >= GNSS_PERIOD){
      latest_gnss = GNSS::getData();
      last_gnss_time += GNSS_PERIOD;
    }
    
    if(current_time - last_sp_monitor_time >= SP_MONITOR_PERIOD){
      latest_sp1_monitor = PowerMonitor::getData();
      latest_sp2_monitor = PowerMonitor::getData();
      last_sp_monitor_time += SP_MONITOR_PERIOD;
    }
    
    //for checking telemetry cycle period
    if(current_time - last_telemetry_time >= TELEMETRY_PERIOD){                
      Telemetry::sendPQ(
        pq_telemetry_node, gs_telemetry_node,
        latest_barometer,
        latest_power_monitor,
        latest_imu,
        latest_magnetometer,
        latest_gnss,
        latest_sp1_monitor,
        latest_sp2_monitor,
        mode, mech_state);
        
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

    printf("\n[PQ] PQ returned %d\n", result);
}
