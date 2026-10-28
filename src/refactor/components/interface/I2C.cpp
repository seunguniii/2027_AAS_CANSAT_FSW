#include "I2C.hpp"

#include <iostream>

namespace I2C{
  esp_err_t Bus::init(const BusConfig& config){    
    i2c_master_bus_config_t bus_config{};
    
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = config.port;
    bus_config.sda_io_num = config.sda_pin;
    bus_config.scl_io_num = config.scl_pin;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = config.internal_pullup;

    return i2c_new_master_bus(&bus_config, &bus_handle); 
  }
  
  
  esp_err_t Bus::addDevice(uint8_t address, uint32_t frequency_hz, i2c_master_dev_handle_t& device){    
    if(handle_ == nullptr) return ESP_ERR_INVALID_STATE;
    
    i2c_device_config_t device_config{};
    
    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = address;
    device_config.scl_speed_hz = frequency_hz;
    
    return i2c_master_bus_add_device(handle_, &device_config, &device);
  }
  
  
  bool readRegisters(i2c_master_dev_handle_t device, uint8_t reg, uint8_t* data, size_t length) {
    esp_err_t err = i2c_master_receive(device, data, length, -1);
    if(err != ESP_OK){
      std::cout << "[I2C] Register read failed: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    return true;
  }
  
  
  bool writeRegisters(i2c_master_dev_handle_t device, uint8_t reg, const uint8_t* data, size_t length){
    uint8_t buffer[257];
    
    if(length > sizeof(buffer) - 1) return false;
    
    buffer[0] = reg;
    
    for(size_t i = 0; i < length; i ++){buffer[i+1] = data[i];}
    
    esp_err_t err = i2c_master_transmit(device, buffer, length + 1, -1);
    if(err != ESP_OK){
      std::cout << "[I2C] Register write failed: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    return true;
  }
}
