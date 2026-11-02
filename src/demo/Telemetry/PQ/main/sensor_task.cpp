#include "app_runtime.hpp"

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Sensors.hpp"

namespace App{
  namespace{
    constexpr char TAG[] = "[SensorTask]";
    
    //sensor periods
    constexpr uint32_t BAROMETER_PERIOD_MS = 50;
    constexpr uint32_t POWER_MONITOR_PERIOD_MS = 100;
    constexpr uint32_t IMU_PERIOD_MS = 50;
    constexpr uint32_t MAGNETOMETER_PERIOD_MS = 100;
    constexpr uint32_t GNSS_PERIOD_MS = 50;
    constexpr uint32_t SP_PERIOD_MS = 100;
  }//namespace
  
  void sensorTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    //wait until all workers are created
    xEventGroupWaitBits(runtime->start_gate, TASKS_START_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
    SensorSnapshot snapshot{};
    
    //sensors
    bool have_all_sensors = false;
    
    bool have_barometer = false;
    bool have_power_monitor = false;
    bool have_imu = false;
    bool have_magnetometer = false;
    bool have_gnss = false;
    bool have_sp = false;
    
    uint32_t now = nowMS();
    
    //set last_<sensor>_ms to now - <sensor>_period_ms to enforce readings from 0ms
    uint32_t last_barometer_ms = now - BAROMETER_PERIOD_MS;
    uint32_t last_power_monitor_ms = now - POWER_MONITOR_PERIOD_MS;
    uint32_t last_imu_ms = now - IMU_PERIOD_MS;
    uint32_t last_magnetometer_ms = now - MAGNETOMETER_PERIOD_MS;
    uint32_t last_gnss_ms = now - GNSS_PERIOD_MS;
    uint32_t last_sp_ms = now - SP_PERIOD_MS;
    
    std::cout << TAG << " SensorTask started." << std::endl;
    
    while(true){
      bool snapshot_updated = false;
      
      now = nowMS();
      if(now - last_barometer_ms >= BAROMETER_PERIOD_MS){
        snapshot.barometer = barometer.getData();
        
        last_barometer_ms = nowMS();
        have_barometer = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_power_monitor_ms >= POWER_MONITOR_PERIOD_MS){
        snapshot.power_monitor = power_monitor.getData();
        
        last_power_monitor_ms = nowMS();
        have_power_monitor = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_imu_ms >= IMU_PERIOD_MS){
        snapshot.imu = imu.getData();
        
        last_imu_ms = nowMS();
        have_imu = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_magnetometer_ms >= MAGNETOMETER_PERIOD_MS){
        snapshot.magnetometer = magnetometer.getData();
        
        last_magnetometer_ms = nowMS();
        have_magnetometer = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_gnss_ms >= GNSS_PERIOD_MS){
        snapshot.gnss = gnss.getData();
        
        last_gnss_ms = nowMS();
        have_gnss = true;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(now - last_sp_ms >= SP_PERIOD_MS){
        snapshot.sp0 = sp0_monitor.getData();
        snapshot.sp1 = sp1_monitor.getData();
        
        last_sp_ms = nowMS();
        have_sp = true;
        snapshot_updated = true;
      }
      //TODO add sensors
      have_all_sensors = (have_barometer && have_power_monitor && have_imu &&
                          have_magnetometer && have_gnss && have_sp); 
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
