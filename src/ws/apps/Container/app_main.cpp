#include <iostream>
#include <cstdio>
#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "I2C.hpp"
#include "BMP581.hpp"
#include "Msg.hpp"
#include "Sensors.hpp"
#include "Telemetry.hpp"

constexpr MCU::ID CTR_MCU = MCU::ID::CTR;
constexpr MCU::ID GS_MCU = MCU::ID::GS;

const Msg::Node ctr_telemetry_node{Node::Address{CTR_MCU, Node::ID::TELEMETRY}};
const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};


int main(void){
  uint32_t current_time = 0;  
  
  //telemetry
  uint32_t last_telemetry_time = 0;
  uint32_t TELEMETRY_PERIOD = 250; //[ms], depends on operation state, default 4Hz
  
  //interface setup
  I2C::Bus I2C_BUS;
  const I2C::BusConfig I2C_CONFIG{
    .port = I2C_NUM_0,
    .sda_pin = GPIO_NUM_17,
    .scl_pin = GPIO_NUM_18,
    .internal_pullup = true
  };
  
  std::cout << "[CTR] I2C Configuration" << std::endl;
  std::cout << "      Port           : " << I2C_CONFIG.port << std::endl;
  std::cout << "      SDA_PIN        : " << I2C_CONFIG.sda_pin << std::endl;
  std::cout << "      SCL_PIN        : " << I2C_CONFIG.scl_pin << std::endl;
  std::cout << "      INTERNAL_PULLUP: " << I2C_CONFIG.internal_pullup << std::endl;
  
  std::cout << "[CTR] Initializing I2C bus..." << std::endl;
  ESP_ERROR_CHECK(I2C_BUS.init(I2C_CONFIG));
  
  //SENSORS
  //bmp581
  constexpr uint8_t BMP581_ADDR = 0x47;
  constexpr uint32_t BMP581_FREQ_HZ = 10000; //10 kHz
  i2c_master_dev_handle_t BMP581_handle = nullptr;
  
  std::cout << "[CTR] Adding BMP581 as an I2C device..." << std::endl;
  ESP_ERROR_CHECK(i2c_bus.addDevice(BMP581_ADDR, BMP581_FREQ+HZ, BMP581_handle);
  
  //TODO other sensors
  
  //sensor timing
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
  
  while(true){
    current_time = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    
    if(BMP581_init_err != ESP_OK) {
      std::cout << "[CTR] BMP581 I2C bus not initialized. Retrying..." << std::endl;
      BMP581_init_err = BMP581_bus.init(BMP581_config);
    }else if(current_time - last_barometer_time >= BAROMETER_PERIOD){
      latest_barometer = Barometer::getData();
      last_barometer_time += BAROMETER_PERIOD;
    }
    
    if(current_time - last_power_monitor_time >= POWER_MONITOR_PERIOD){
      latest_power_monitor = PowerMonitor::getData();
      last_power_monitor_time += POWER_MONITOR_PERIOD;
    }
    
    //check telemetry cycle period
    if(current_time - last_telemetry_time >= TELEMETRY_PERIOD){
      Telemetry::sendCTR(
        ctr_telemetry_node, gs_telemetry_node,
        latest_barometer, latest_power_monitor,
        mode, state, mech_state);
        
      last_telemetry_time += TELMETRY_PERIOD;
    }
    
    //TODO
    //if(landed) return 0;      
    
    vTaskDelay(pdMS_TO_TICKS(1));
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
