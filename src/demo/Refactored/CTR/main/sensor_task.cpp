#include "app_runtime.hpp"

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Sensors.hpp"

namespace App{
  namespace{
    constexpr char TAG[] = "[SensorTask]";
    
    //sensor periods
    constexpr int64_t BAROMETER_PERIOD_MS = 50;
    constexpr int64_t POWER_MONITOR_PERIOD_MS = 100;
  }//namespace
  
  void sensorTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    //wait until all workers are created
    xEventGroupWaitBits(runtime->start_gate, TASKS_START_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
    SensorSnapshot snapshot{};
    
    //TODO add sensors
    bool have_all_sensors = false;
    bool have_barometer = false;
    bool have_power_monitor = false;
    
    int64_t now = nowMS();
    
    //set last_<sensor>_ms to now - <sensor>_period_ms to enforce readings from 0ms
    int64_t last_barometer_ms = now - BAROMETER_PERIOD_MS;
    int64_t last_power_monitor_ms = now - POWER_MONITOR_PERIOD_MS;
    
    std::cout << TAG << " SensorTask started." << std::endl;
    
    while(true){
      bool snapshot_updated = false;
      
      now = nowMS();
      if(now - last_barometer_ms >= BAROMETER_PERIOD_MS){
        snapshot.barometer = Barometer::getData();
        
        last_barometer_ms = nowMS();
        have_barometer = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_power_monitor_ms >= POWER_MONITOR_PERIOD_MS){
        snapshot.power_monitor = PowerMonitor::getData();
        
        last_power_monitor_ms = nowMS();
        have_power_monitor = true;
        snapshot_updated = true;
      }
      
      //TODO add sensors
      have_all_sensors = (have_barometer && have_power_monitor); 
      if(snapshot_updated && have_all_sensors){
        snapshot.captured_at_ms = nowMS();
        
        if(xQueueOverwrite(runtime->sensor_snapshot_queue, &snapshot) == pdPASS)
          runtime->last_snapshot_published_ms.store(snapshot.captured_at_ms);
      }  
      //time after a cycle of sensor task
      runtime->sensor_heartbeat_ms.store(nowMS()); 
      vTaskDelay(1);
    }
  }
}//namespace App
