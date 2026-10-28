#include "Sensors.hpp"
#include "I2C.hpp"
#include "Msg.hpp"

#include <iostream>
#include <cstdint>

#include "esp_timer.h"


namespace Barometer{
  namespace{
    constexpr char TAG[] = "BMP581";
    
    constexpr uint8_t BMP581_ADDRESS = 0x47;
    
    constexpr uint8_t REG_CHIP_ID = 0x01;
    constexpr uint8_t REG_INT_STATUS = 0x27;    
    constexpr uint8_t REG_TEMP_DATA = 0x1D;
    constexpr uint8_t REG_OSR_CONFIG = 0x36;
    constexpr uint8_t REG_ODR_CONFIG = 0x37;
    
    //ODR 20Hz  = 0x15 //ref. Bosch ODR table
    //pwr_mode  = 0x01 //NORMAL
    //0x15 << 2 = 0x54
    constexpr uint8_t ODR_CONFIG_20HZ_NORMAL = 0x55;
    
    i2c_master_dev_handle_t dvice_handle = nullptr;
    
    bool readRegisters(uint8_t reg, uint8_t* data, length){return I2C::readRegisters(device_handle, reg, data, length);}
    bool writeRegisters(uint8_reg, const uint8_t* data, size_t length){return I2C::writeRegisters(device_handle, reg, data, length);}
    
    //constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
    
    //constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
    //constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
    
    constexpr uint32_t I2C_FREQ_HZ = 100000;
    
    
  }
  
  bool init(I2C::Bus& bus) {
    std::cout << "[BMP581] Starting sensor initialization..." << std::endl;
    
    esp_err_t err = bus.addDevice(BMP581_ADDRESS, I2C_FREQ_HZ, device_handle);
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
    if(chip_id != 0x50){
      std::cout << "[BMP581] Unexpected chip ID: 0x%02X", chip_id);
      return false;
    }
    std::cout << "[BMP581] Chip ID: 0x" << std::hex << static_cast<int>(chip_id) << std::dec << std::endl;
    
    //sensor config
    std::cout << "[BMP581] Configuring sensor..." << std::endl;
    //enable pressure reading
    //pressure enabled = 0x40
    //OSR_P            = 0x20
    //OSR_T            = 0x04
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
  

  Sensor::Barometer getData(void){
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
}
