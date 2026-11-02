#pragma once

#include <cstdint>
#include <cstddef>
#include "States.hpp"
#include "I2C.hpp"

namespace Sensor{
  struct __attribute__((__packed__)) SensorHeader{
    uint32_t timestamp; //[ms]		system/mission time since boot	4bytes
    Health health;	//		sensor health/reliability	1byte
  };

  struct __attribute__((__packed__)) IMU{
    SensorHeader header;//		sensor header			5bytes
    float accel[3];	//[m/s^2]	acceleration	right back down	12bytes
    float rot_rate[3];	//[deg/s] 	rotation rates	roll pitch yaw	12bytes
    float q[4];		//		quaternion	x y z w		16bytes
  }; 			//		struct imu			45bytes
  
  struct __attribute__((__packed__)) Barometer{
    SensorHeader header;//		sensor header			5bytes
    float altitude;	//[m]		altitude, 0.1m res		4bytes
    float pressure;	//[pa]		air pressure, 1pa res		4bytes
    float temperature; 	//[C*]		temperature, 0.1 *C res		4bytes
  };			//		struct barometer		17bytes
  
  struct __attribute__((__packed__)) PowerMonitor{
    SensorHeader header;//		sensor header			5bytes
    float voltage;	//[V]		0.1V res			4bytes
    float current;	//[mA]		1mA res				4bytes
  };			//		struct power monitor		13bytes
  
  struct __attribute__((__packed__)) Magnetometer{
    SensorHeader header;//		sensor header			5bytes
    float mag[3];	//[mG]		east south down, 1mG res	12bytes
  };			//		struct magnetometer		17bytes

  
  struct __attribute__((__packed__)) GNSS{
    SensorHeader header;//		sensor header			5bytes
    uint32_t gnss_time;	//[default]	gnss provided time		4bytes
    float position[3];	//[deg deg m]	lat lon alt			12bytes
    uint8_t sats;	//		# of sats			1byte
  };			//		struct gnss			22bytes
  
  
}//namespace Sensor


namespace IMU{
  class BNO085{
    public:
      Sensor::IMU getData();
    private:
  };
}

namespace Barometer{
  class BMP581{
    public:
      bool init(I2C::Bus& bus, uint8_t address);
      Sensor::Barometer getData();
    
    private:
      i2c_master_dev_handle_t device_handle = nullptr;
      
      static constexpr uint8_t REG_CHIP_ID = 0x01;
      static constexpr uint8_t REG_INT_STATUS = 0x27;    
      static constexpr uint8_t REG_TEMP_DATA = 0x1D;
      static constexpr uint8_t REG_OSR_CONFIG = 0x36;
      static constexpr uint8_t REG_ODR_CONFIG = 0x37;
      
      //ODR 20Hz  = 0x15 //ref. Bosch ODR table
      //pwr_mode  = 0x01 //NORMAL
      //0x15 << 2 = 0x54
      static constexpr uint8_t ODR_CONFIG_20HZ_NORMAL = 0x55;
      
      static constexpr uint32_t I2C_FREQ_HZ = 100000;
      
      bool readRegisters(uint8_t reg, uint8_t* data, size_t length);
      bool writeRegisters(uint8_t reg, const uint8_t* data, size_t lenght);
  };
}

namespace PowerMonitor{
  class INA3221{
    public:
      Sensor::PowerMonitor getData();
    private:
  };
}

namespace Magnetometer{
  class MagSens{
    public:
      Sensor::Magnetometer getData();
    private:
  };
}

namespace GNSS{
  class Gnss{
    public:
      Sensor::GNSS getData();
    private:
  };
}
