#include <iostream>
#include <cstdint>
#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Msg.h"
#include "Nodes.h"
#include "Sensors.h"

//MAIN
int main()
{
  if(!Barometer::begin()){
    std::cout << "[Main] Bus init failed\n";
    return 1;
  }

  Sensor::Barometer barometer;  
  while(true){
    barometer = Barometer::getData();
    std::cout << "[Main] Time        : " << barometer.header.timestamp << std::endl;
    std::cout << "       Health      : " << static_cast<int>(barometer.header.health) << std::endl;
    std::cout << "       Pressure    : " << barometer.pressure << std::endl;
    std::cout << "       Temperature : " << barometer.temperature << std::endl;
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  
  return 0;
}

extern "C" void app_main()
{
    const int result = main();

    std::cout.flush();
    fflush(stdout);
}
