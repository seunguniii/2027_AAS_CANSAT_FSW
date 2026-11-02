#include "Sensors.hpp"
#include "I2C.hpp"

#include <iostream>
#include <cstdint>

#include "esp_timer.h"


namespace Barometer{
  bool BMP581::init(I2C::Bus& bus, uint8_t address) {
    /*
    add bmp581 to i2c bus
    input: pre-configured i2c bus
    output: init?
    */
    std::cout << "[BMP581] Starting sensor initialization..." << std::endl;
    
    esp_err_t err = bus.addDevice(address, I2C_FREQ_HZ, device_handle);
    if(err != ESP_OK){
      std::cout << "[BMP581] i2c_master_bus_add_device error: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    std::cout << "[BMP581] I2C device registeration success." << std::endl;    

    uint8_t chip_id = 0;
    if(!readRegisters(REG_CHIP_ID, &chip_id, 1)){
      std::cout << "[BMP581] Failed to read chip ID" << std::endl;
      return false;
    }    
    if(chip_id != 0x50){ //checks if the chip is BMP581
      std::cout << "[BMP581] Unexpected chip ID: " << chip_id << std::endl;
      return false;
    }
    std::cout << "[BMP581] Chip ID: 0x" << std::hex << static_cast<int>(chip_id) << std::dec << std::endl;
        
    //sensor config
    std::cout << "[BMP581] Configuring sensor..." << std::endl;
    //enable pressure reading
    //pressure enabled = 0x40
    //OSR_P            = 0x20
    //OSR_T            = 0x04
    //------------------------
    uint8_t osr_config = 0x64;
    if(!writeRegisters(REG_OSR_CONFIG, &osr_config, 1)) return false;
    std::cout << "[BMP581] OSR configured." << std::endl;
        
    //20hz normal
    uint8_t odr_config = ODR_CONFIG_20HZ_NORMAL;
    if(!writeRegisters(REG_ODR_CONFIG, &odr_config, 1)) return false;
    std::cout << "[BMP581] ODR configured." << std::endl;
        
    std::cout << "[BMP581] Configuration complete." << std::endl;
    std::cout << "         OSR Pressure    : 16x" << std::endl;
    std::cout << "         OSR Temperature : 16x" << std::endl;
    std::cout << "         ODR : 20Hz NORMAL" << std::endl;
        
    std::cout << "[BMP581] Standby." << std::endl;
    return true;    
  }
      
  Sensor::Barometer BMP581::getData(void){
    /*
    get data from bmp581
    output: barometer data    
    */
    Sensor::Barometer barometer{};
    
    uint8_t data[6] = {};
    
    if(!readRegisters(REG_TEMP_DATA, data, sizeof(data))){
      barometer.header.health = Sensor::Health::FAULT;
      return barometer;
    }
    int32_t temperature_raw = (static_cast<int32_t>(static_cast<int8_t>(data[2])) << 16) |
                              (static_cast<int32_t>(data[1])) << 8 |
                               static_cast<int32_t>(data[0]);                          
    int32_t pressure_raw = (static_cast<int32_t>(static_cast<int8_t>(data[5])) << 16) |
                           (static_cast<int32_t>(data[4])) << 8 |
                            static_cast<int32_t>(data[3]);    
                              
    float temperature = static_cast<float>(temperature_raw) / 65536.0f;
    float pressure = static_cast<float>(pressure_raw) / 64.0f;    
    
    //fill sensor data
    //esp_timer_get_time gives [us]; convert to [ms]
    barometer.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    barometer.header.health = Sensor::Health::HEALTHY;
    barometer.altitude = 123.0; //dummy TODO
    barometer.pressure = pressure;    
    barometer.temperature = temperature;
  
    return barometer;
  }

  bool BMP581::readRegisters(uint8_t reg, uint8_t* data, size_t length){return I2C::readRegisters(device_handle, reg, data, length);}
  bool BMP581::writeRegisters(uint8_t reg, const uint8_t* data, size_t length){return I2C::writeRegisters(device_handle, reg, data, length);}
}//namepsace Barmometer
