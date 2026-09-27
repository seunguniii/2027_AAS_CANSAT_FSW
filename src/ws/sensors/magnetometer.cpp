#include "Sensors.h"

#include "esp_timer.h"

#include "Msg.h"

namespace Magnetometer{
  //TODO command parser for simulation mode
  //CMD::CTR getCMD(void){}


  Sensor::Magnetometer getData(void){
    Sensor::Magnetometer magnetometer{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    magnetometer.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    magnetometer.header.health = Sensor::Health::HEALTHY; //dummy
  
    //dummy data
    magnetometer.mag[0] = 7;
    magnetometer.mag[1] = 12;
    magnetometer.mag[2] = 5;
  
    return magnetometer;
  }
}
