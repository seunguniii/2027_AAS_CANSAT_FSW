#pragma once

#include <cstddef>
#include <cstdint>

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

namespace I2C{
  struct BusConfig{
    i2c_port_num_t port;
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    
    bool internal_pullup;
  }
  
  class Bus{
    public:
      esp_err_t addDevice(uint8_t address,
                          uint32_t frequency_hz,
                          i2c_master_dev_handle_t& device);

    private:
      i2c_master_bus_handle_t handle_ = nullptr;
  };
  
  bool readRegisters(i2c_master_dev_handle_t device, uint8_reg, const uint8_t* data, size_t length);
  bool writeRegisters(i2c_master_dev_handle_t device, uint8_reg, const uint8_t* data, size_t length);
}

