#include <iostream>
#include <cstdio>
#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MCU.hpp"
#include "Msg.hpp"
#include "MsgESPNOW.hpp"
#include "Sensors.hpp"
#include "Telemetry.hpp"

constexpr MCU::ID CTR_MCU = MCU::ID::ESP1;
constexpr MCU::ID GS_MCU = MCU::ID::ESP_CAM_0;

const Msg::Node ctr_telemetry_node{Node::Address{CTR_MCU, Node::ID::TELEMETRY}};
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};


//I2C CONFIG    
constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
static I2C::Bus i2c_bus;
I2C::BusConfig i2c_config{
  .port = I2C_PORT,
  .sda_pin = SDA_PIN,
  .scl_pin = SCL_PIN,
  .internal_pullup = true
};


//MAIN
extern "C" void app_main()
{
  std::cout << "[CTR] Initializing...\n";
  
  if(!Msg::detail::beginESPNOW()){
    std::cout << "[CTR] ESP-NOW initialization failed.\n";
    return;
  }
  
  
  uint32_t current_time = 0;  
  
  //telemetry
  uint32_t last_telemetry_time = 0;
  uint32_t TELEMETRY_PERIOD = 250; //[ms], depends on operation state, default 4Hz
  
  //interface
  //initialize I2C bus
  esp_err_t err = i2c_bus.init(i2c_config);
  if(err != ESP_OK) {
    std::cout << "[CTR] I2C bus initialization failed." << esp_err_to_name(err) << std::endl;
    return;
  }
  std::cout << "[CTR] I2C bus initialization success." << std::endl;
  std::cout << "[CTR] I2C Configuration" << std::endl;
  std::cout << "       Port           : " << i2c_config.port << std::endl;
  std::cout << "       SDA_PIN        : " << i2c_config.sda_pin << std::endl;
  std::cout << "       SCL_PIN        : " << i2c_config.scl_pin << std::endl;
  std::cout << "       INTERNAL_PULLUP: "
            << std::boolalpha << i2c_config.internal_pullup
            << std::endl;
  
  
  //sensors
  //BMP581
  Sensor::Barometer latest_barometer{};  
  if(!Barometer::init(i2c_bus)){
    std::cout << "[CTR] BMP581 initialization failed." << std::endl;
    return;
  }
  std::cout << "[CTR] BMP581 initialization success." << std::endl;
  
  
  Sensor::PowerMonitor latest_power_monitor{};
  
  uint32_t last_barometer_time = 0;
  uint32_t last_power_monitor_time = 0;
  constexpr uint32_t BAROMETER_PERIOD = 50; //[ms], 20Hz
  constexpr uint32_t POWER_MONITOR_PERIOD = 100;//[ms], 10Hz
  
  //states
  FlightMode mode = FlightMode::SIMULATION;
  CTR_OpState state = CTR_OpState::LAUNCH_PAD;
  uint8_t mech_state = 0x00;
  
  //TODO Video::start_rec();
  
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
    
    vTaskDelay(pdMS_TO_TICKS(1));
  }
  return;
}
