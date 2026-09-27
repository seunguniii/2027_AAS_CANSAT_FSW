#pragma once

#include <cstdint>
#include "States.h"

namespace Sensor{
  struct __attribute__((__packed__)) SensorHeader{
    uint32_t timestamp; //[ms]		system/mission time since boot	4bytes
    SensorHealth health;//		sensor health/reliability	1byte
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
    float temperature; 	//[C*]		temperature, 0.1C* res		4bytes
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
