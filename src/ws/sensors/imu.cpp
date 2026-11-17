#include "Sensors.h"

#include "esp_timer.h"

#include "Msg.h"

namespace IMU{
  //TODO command parser for simulation mode
  //CMD getCMD(void){}


  Sensor::IMU getData(void){
    Sensor::IMU imu{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    imu.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    imu.header.health = Sensor::Health::HEALTHY; //dummy TODO Sensors.checkHealth(raw_data);
  
    //dummy data
    imu.accel[0] = 0.5; //east
    imu.accel[1] = -0.75; //south
    imu.accel[2] = 0.1; //down
    
    imu.rot_rate[0] = 1.5; //roll
    imu.rot_rate[1] = 3.14; //pitch
    imu.rot_rate[2] = -5.6; //yaw
    
    imu.q[0] = 0.0; //x
    imu.q[1] = 0.0; //y
    imu.q[2] = 0.0; //z
    imu.q[3] = 1.0; //w
  
    return imu;
  }
}
