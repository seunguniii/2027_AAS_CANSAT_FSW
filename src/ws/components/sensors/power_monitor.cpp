#include "Sensors.hpp"

#include <iostream>
#include <cmath>
#include <cstdint>

#include "esp_timer.h"
#include "esp_rom_sys.h"

#include "Endians.hpp"

//#include "Msg.hpp"

namespace PowerMonitor{
  bool INA228::read16(uint8_t reg, uint16_t& value) {
    uint8_t data[2] = {};
    if (!readRegisters(reg, data, sizeof(data))) return false;
    value = utils::readUInt16BE(data);
    return true;
  }
  bool INA228::write16(uint8_t reg, uint16_t value) {
    uint8_t data[2] = {static_cast<uint8_t>(value >> 8),
                       static_cast<uint8_t>(value & 0xFF)};
    return writeRegisters(reg, data, sizeof(data));
  }
  
  
  bool INA228::init(I2C::Bus& bus, uint8_t address){
    /*
    add ina228 to i2c bus
    input: pre-configured i2c bus
    output: init?
    */
    esp_err_t err = bus.addDevice(address, I2C_FREQ_HZ, device_handle);

    if(err != ESP_OK){
      std::cout << "[INA228] I2C device registration failed: "  << esp_err_to_name(err) << std::endl;
      return false;
    }
    std::cout << "[INA228] I2C registration successful." << std::endl;


    //verify manufact. id
    uint16_t manufacturer_id = 0;
    if(!read16(REG_MANUFACTURER, manufacturer_id)) {
      std::cout << "[INA228] Failed to read manufacturer ID." << std::endl;
      return false;
    }
    if(manufacturer_id != EXPECTED_MANUFACTURER_ID){
      std::cout << "[INA228] Unexpected manufacturer ID: 0x" << std::hex << manufacturer_id << std::dec << std::endl;
      return false;
    }
    std::cout << "[INA228] Manufacturer ID verified." << std::endl;


    //verify device id
    uint16_t device_id = 0;
    if(!read16(REG_DEVICE_ID, device_id)) {
      std::cout << "[INA228] Failed to read device ID." << std::endl;
        return false;
    }
    //upper 12 bits : INA228.
    //lower 4 bits : revision.
    if ((device_id >> 4) != 0x228){
      std::cout << "[INA228] Unexpected device ID: 0x" << std::hex << device_id << std::dec << std::endl;
      return false;
    }
    std::cout << "[INA228] Device ID: 0x" << std::hex << device_id << std::dec << std::endl;


    std::cout << "[INA228] Resetting sensor..." << std::endl;
    // CONFIG register bit 15 = RST
    if(!write16(REG_CONFIG, 0x8000)){
      std::cout << "[INA228] Reset failed." << std::endl;
      return false;
    }
    esp_rom_delay_us(2000); //grace


    std::cout << "[INA228] Configuring shunt calibration..." << std::endl;

    //lsb; max expect current / 2^19
    current_lsb_A = MAX_EXPECTED_CURRENT_A / 524288.0f;

    //SHUNT_CAL = 13107.2 * 10^6 * CURRENT_LSB * R_SHUNT
    //ADCRANGE default 0 after reset.
    const float shunt_cal_float = 13107.2e6f * current_lsb_A * SHUNT_RESISTANCE_OHM;
    const uint16_t shunt_cal = static_cast<uint16_t>(std::lround(shunt_cal_float));
    if(shunt_cal == 0 || shunt_cal > 0x7FFF){
      std::cout << "[INA228] Invalid shunt calibration value." << std::endl;
      return false;
    }
    if(!write16(REG_SHUNT_CAL, shunt_cal)){
      std::cout << "[INA228] Failed to write SHUNT_CAL." << std::endl;
      return false;
    }
    std::cout << "[INA228] SHUNT_CAL = " << shunt_cal << std::endl;
              
              
    std::cout << "[INA228] Configuring ADC..." << std::endl;
    if (!write16(REG_ADC_CONFIG, ADC_CONFIG_VALUE)) {
      std::cout << "[INA228] Failed to configure ADC." << std::endl;
      return false;
    }
    esp_rom_delay_us(5000); //grace

    std::cout << "[INA228] Initialization complete." << std::endl;
    return true;
  }
  
  
  Sensor::PowerMonitor INA228::getData(void){
    Sensor::PowerMonitor power_monitor{};

    uint8_t vbus_data[3] = {};
    uint8_t vshunt_data[3] = {};
    uint8_t current_data[3] = {};

    if(!readRegisters(REG_VBUS, vbus_data, sizeof(vbus_data)) ||
       !readRegisters(REG_VSHUNT, vshunt_data, sizeof(vshunt_data)) ||
       !readRegisters(REG_CURRENT, current_data, sizeof(current_data))){
      power_monitor.header.health = State::Health::FAULT;
      return power_monitor;
    }

    //big endian reg data
    uint32_t vbus_raw = utils::readUInt24BE(vbus_data);
    uint32_t vshunt_raw = utils::readUInt24BE(vshunt_data);
    uint32_t current_raw = utils::readUInt24BE(current_data);

    //extract 20bit measurement values (23:4)
    uint32_t vbus_code = vbus_raw >> 4;

    int32_t vshunt_code = static_cast<int32_t>(vshunt_raw >> 4);
    int32_t current_code = static_cast<int32_t>(current_raw >> 4);

    //signed 20bit measurements
    if (vshunt_code & 0x80000) vshunt_code -= 0x100000;
    if (current_code & 0x80000) current_code -= 0x100000;

    //vbus = 195.3125 uV / LSB
    float voltage = static_cast<float>(vbus_code) * 195.3125e-6f;

    //vshunt = 312.5 nV / LSB (ADCRANGE = 0)
    float shunt_voltage = static_cast<float>(vshunt_code) * 312.5e-9f;

    //current = current_lsb_A config'd during init
    float current = static_cast<float>(current_code) * current_lsb_A * 1000;

    //power
    float measured_power = voltage * current;

    power_monitor.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    power_monitor.header.health = State::Health::HEALTHY;

    power_monitor.voltage = voltage;
    power_monitor.current = current;
    power_monitor.power = measured_power;

    return power_monitor;
  }

  bool INA228::readRegisters(uint8_t reg, uint8_t* data, size_t length){return I2C::readRegisters(device_handle, reg, data, length);}
  bool INA228::writeRegisters(uint8_t reg, const uint8_t* data, size_t length){return I2C::writeRegisters(device_handle, reg, data, length);}
}
