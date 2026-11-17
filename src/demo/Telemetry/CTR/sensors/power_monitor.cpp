#include "Sensors.h"

#include "esp_timer.h"

#include "Msg.h"

namespace PowerMonitor{
  //TODO command parser for simulation mode
  //CMD::CTR getCMD(void){}


  Sensor::PowerMonitor getData(void){
    Sensor::PowerMonitor power_monitor{};
  
    //header
    //esp_timer_get_time gives [us]; convert to [ms]
    power_monitor.header.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    power_monitor.header.health = Sensor::Health::HEALTHY; //dummy
  
    //dummy data
    power_monitor.voltage = 4.2;
    power_monitor.current = 2.1;
  
    return power_monitor;
  }
}
