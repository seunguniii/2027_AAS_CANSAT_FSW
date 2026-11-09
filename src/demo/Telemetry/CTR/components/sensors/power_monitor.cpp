#include "Sensors.hpp"

#include "esp_timer.h"

//#include "Msg.hpp"

namespace PowerMonitor{  
  bool INA228::init(I2C::Bus& bus, uint8_t address){
    /*
    add bmp581 to i2c bus
    input: pre-configured i2c bus
    output: init?
    */
    std::cout << "[INA228] Starting sensor initialization..." << std::endl;
    
    esp_err_t err = bus.addDevice(address, I2C_FREQ_HZ, device_handle);
    if(err != ESP_OK){
      std::cout << "[INA228] i2c_master_bus_add_device error: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    std::cout << "[INA228] I2C device registeration success." << std::endl;    

    uint8_t chip_id = 0;
    if(!readRegisters(REG_CHIP_ID, &chip_id, 1)){
      std::cout << "[INA228] Failed to read chip ID" << std::endl;
      return false;
    }    
    if(chip_id != 0x50){ //TODO change to INA228 chip ID
      std::cout << "[INA228] Unexpected chip ID: " << chip_id << std::endl;
      return false;
    }
    std::cout << "[INA228] Chip ID: 0x" << std::hex << static_cast<int>(chip_id) << std::dec << std::endl;
    
    //TODO sensor config
        
    std::cout << "[INA228] Standby." << std::endl;
    return true;    
  }
  Sensor::PowerMonitor INA228::getData(void){
    Sensor::PowerMonitor power_monitor{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    power_monitor.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    power_monitor.header.health = Sensor::Health::HEALTHY; //dummy
  
    //dummy data
    power_monitor.voltage = 4.2;
    power_monitor.current = 2.1;
  
    return power_monitor;
  }
}
