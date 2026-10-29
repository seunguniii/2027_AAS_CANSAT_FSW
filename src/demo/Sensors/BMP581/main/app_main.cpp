#include <iostream>
#include <cstdint>
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "I2C.hpp"
#include "Sensors.hpp"

extern "C" void app_main()
{
  constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
    
  constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
  constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
  
  static I2C::Bus i2c_bus;
  I2C::BusConfig i2c_config{
    .port = I2C_PORT,
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .internal_pullup = true
  };
  
  std::cout << "[Main] I2C Configuration" << std::endl;
  std::cout << "       Port           : " << i2c_config.port << std::endl;
  std::cout << "       SDA_PIN        : " << i2c_config.sda_pin << std::endl;
  std::cout << "       SCL_PIN        : " << i2c_config.scl_pin << std::endl;
  std::cout << "       INTERNAL_PULLUP: "
            << std::boolalpha << i2c_config.internal_pullup
            << std::endl;

  // Initialize the I2C bus
  esp_err_t err = i2c_bus.init(i2c_config);
  if(err != ESP_OK) {
    std::cout << "[Main] I2C bus initialization failed." << esp_err_to_name(err) << std::endl;
    return;
  }
  std::cout << "[Main] I2C bus initialization success." << std::endl;
  
  if(!Barometer::init(i2c_bus)){
    std::cout << "[Main] BMP581 initialization failed." << std::endl;
    return;
  }
  std::cout << "[Main] BMP581 initialization success." << std::endl;
  
  Sensor::Barometer barometer;  
  
  while(true){
    barometer = Barometer::getData();
    std::cout << "[Main] Time        : " << barometer.header.timestamp << std::endl;
    std::cout << "       Health      : " << static_cast<int>(barometer.header.health) << std::endl;
    std::cout << "       Pressure    : " << barometer.pressure << std::endl;
    std::cout << "       Temperature : " << barometer.temperature << std::endl;
    
    vTaskDelay(pdMS_TO_TICKS(50)); //20Hz
  }
  
  return;
}
