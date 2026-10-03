#include "Sensors.h"

#include <iostream>
#include <cstddef>

#include "esp_timer.h"
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Msg.h"

namespace Barometer{
  namespace{
    constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
    
    constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
    constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
    
    constexpr uint32_t I2C_FREQ_HZ = 100000;
    constexpr uint8_t BMP581_ADDRESS = 0x47;
    
    i2c_master_bus_handle_t bus_handle = nullptr;
    i2c_master_dev_handle_t bmp581_handle = nullptr;
    
    constexpr uint8_t REG_CHIP_ID = 0x01;
    constexpr uint8_t REG_INT_STATUS = 0x27;    
    constexpr uint8_t REG_TEMP_DATA = 0x1D;
    constexpr uint8_t REG_OSR_CONFIG = 0x36;
    constexpr uint8_t REG_ODR_CONFIG = 0x37;
    
    //ODR 20Hz  = 0x15 //ref. Bosch ODR table
    //pwr_mode  = 0x01 //NORMAL
    //0x15 << 2 = 0x54
    constexpr uint8_t ODR_CONFIG_20HZ_NORMAL = 0x55;
  }
  
  bool begin() {
    //I2C setup
    std::cout << "[BMP581] Starting I2C initialization..." << std::endl;
    i2c_master_bus_config_t bus_config{};
    
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = I2C_PORT;
    bus_config.sda_io_num = SDA_PIN;
    bus_config.scl_io_num = SCL_PIN;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;
    
    esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle);
    if(err != ESP_OK){
      std::cout << "[BMP581] i2c_new_master_bus error: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    
    i2c_device_config_t device_config{};
    
    device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device_config.device_address = BMP581_ADDRESS;
    device_config.scl_speed_hz = I2C_FREQ_HZ;
    
    err = i2c_master_bus_add_device(bus_handle, &device_config, &bmp581_handle);
    if(err != ESP_OK){
      std::cout << "[BMP 581] i2c_master_bus_add_device error: " << esp_err_to_name(err) << std::endl;
      bus_handle = nullptr;
      return false;
    }
    
    std::cout << "[BMP581] I2C initialization success." << std::endl;
    
    
    //verify sensor
    std::cout << "[BMP581] Starting sensor validation..." << std::endl;
    uint8_t chip_id = 0;
    if(!readRegisters(REG_CHIP_ID, &chip_id, 1)) return false;
    
    std::cout << "[BMP581] Chip ID: 0x" << std::hex << static_cast<int>(chip_id) << std::dec << std::endl;
    
    if(chip_id != 0x50){
      std::cout << "[BMP581] Unexpected chip ID\n";
      return false;
    }
    std::cout << "[BMP581] Sensor validation success." << std::endl;
    
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
  
  
  bool readRegisters(uint8_t reg, uint8_t* data, size_t length) {
    esp_err_t err = i2c_master_transmit(bmp581_handle, &reg, 1, -1);
    if(err != ESP_OK){
      std::cout << "[BMP581] Register write failed: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    
    err = i2c_master_receive(bmp581_handle, data, length, -1);
    if(err != ESP_OK){
      std::cout << "[BMP581] Register read failed: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    return true;
  }
  
  
  bool writeRegisters(uint8_t reg, const uint8_t* data, size_t length){
    uint8_t buffer[1 + 4];
    
    if(length > 4) return false;
    
    buffer[0] = reg;
    
    for(size_t i = 0; i < length; i ++){buffer[i+1] = data[i];}
    
    esp_err_t err = i2c_master_transmit(bmp581_handle, buffer, length + 1, -1);
    if(err != ESP_OK){
      std::cout << "[BMP581] Register write failed: " << esp_err_to_name(err) << std::endl;
      return false;
    }
    return true;
  }
  
  
  bool test(){
    return true;
  }
  
  //TODO command parser for simulation mode
  //CMD::CTR getCMD(void){}


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
