#include "Sensors.h"

#include "esp_timer.h"

#include "Msg.h"

namespace Barometer{
  //TODO command parser for simulation mode
  //CMD::CTR getCMD(void){}


  Sensor::Barometer getData(void){
    Sensor::Barometer barometer{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    barometer.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    barometer.header.health = Sensor::Health::HEALTHY; //dummy
  
    //dummy data
    barometer.altitude = 123.0;
    barometer.pressure = 1023.45;
    barometer.temperature = 23.1;
  
    return barometer;
  }
}
