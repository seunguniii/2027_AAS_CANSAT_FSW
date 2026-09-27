#include "Sensors.h"

#include "esp_timer.h"

#include "Msg.h"

namespace GNSS{
  //TODO command parser for simulation mode
  //CMD getCMD(void){}


  Sensor::GNSS getData(void){
    Sensor::GNSS gnss{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    gnss.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    gnss.header.health = Sensor::Health::HEALTHY; //dummy TODO Sensors.checkHealth(raw_data);
  
    //dummy data
    gnss.gnss_time = gnss.header.timestamp;
    
    gnss.position[0] = 128; //LON
    gnss.position[1] = 38; //LAT
    gnss.position[2] = 952; //ALT
    
    gnss.sats = 7;
  
    return gnss;
  }
}
