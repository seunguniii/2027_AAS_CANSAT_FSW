#include "app_runtime.hpp"

#include <iostream>
#include <cmath>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Sensors.hpp"

namespace{
  constexpr char TAG[] = "[SensorTask]";
    
  //sensor periods
  constexpr int64_t BAROMETER_PERIOD_MS = 50;
  constexpr int64_t POWER_MONITOR_PERIOD_MS = 100;
  constexpr int64_t IMU_PERIOD_MS = 50;
  //constexpr int64_t IR_LID_PERIOD_MS = 50;
  //constexpr int64_t IR_PQ_PERIOD_MS = 50;
  //cam doesn't need a period i suppose?
   
  constexpr int64_t INIT_RETRY_INTERVAL_MS = 1000;
}//namespace  
  
namespace INIT{
  bool BMP581(App::RuntimeContext& runtime){
    if(!App::barometer.init(runtime.i2c_bus, 0x47)){
      std::cout << TAG << " BMP581 initialization failed." << std::endl;
      return false;
    }
    std::cout << TAG << " BMP581 initialized." << std::endl;
    return true;
  }//bmp581
  
  bool INA228(App::RuntimeContext& runtime){
    if(!App::power_monitor.init(runtime.i2c_bus, 0x40)){
      std::cout << TAG << " INA228 initialization failed." << std::endl;
      return false;
    }
    std::cout << TAG << " INA228 initialized." << std::endl;
    return true;
  }
  
  bool BNO085(){return true;} //TODO
  
  bool IR_LID(){return true;} //TODO
  bool IR_PQ(){return true;} //TODO
  
  bool OV2640(){return true;} //TODO
}
  
namespace App{  
  void sensorTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    SensorSnapshot snapshot{};
    
    //initialized?
    bool init_barometer = false;
    bool init_power_monitor = false;
    bool init_imu = false;
    bool init_ir_lid = false;
    bool init_ir_pq = false;
    bool init_cam = false;
    
    int64_t next_barometer_init_ms = 0;
    int64_t next_power_monitor_init_ms = 0;
    int64_t next_imu_init_ms = 0;
    int64_t next_ir_lid_init_ms = 0;
    int64_t next_ir_pq_init_ms = 0;
    int64_t next_cam_init_ms = 0;
    
    //TODO add sensors
    bool have_barometer = false;
    bool have_power_monitor = false;
    bool have_imu = false;
    
    int64_t now = nowMS();
    
    //set last_<sensor>_ms to now - <sensor>_period_ms to enforce readings from 0ms
    int64_t last_barometer_ms = now - BAROMETER_PERIOD_MS;
    int64_t last_power_monitor_ms = now - POWER_MONITOR_PERIOD_MS;
    int64_t last_imu_ms = now - IMU_PERIOD_MS;
    
    std::cout << TAG << " SensorTask started." << std::endl;
    
    while(true){
      if (!init_barometer && now >= next_barometer_init_ms) {
        init_barometer = INIT::BMP581(*runtime);
        if (!init_barometer) next_barometer_init_ms = now + INIT_RETRY_INTERVAL_MS;
      }
         
      if(!init_imu && now >= next_imu_init_ms) init_imu = INIT::BNO085();
      
      if(!init_power_monitor && now >= next_power_monitor_init_ms) {
        init_power_monitor = INIT::INA228(*runtime);
        if(!init_power_monitor) next_power_monitor_init_ms = now + INIT_RETRY_INTERVAL_MS;
      }
      
      if(!init_ir_lid && now >= next_ir_lid_init_ms) init_ir_lid = INIT::IR_LID();
      if(!init_ir_pq && now >= next_ir_pq_init_ms) init_ir_pq = INIT::IR_PQ();
      if(!init_cam && now >= next_cam_init_ms) init_cam = INIT::OV2640();
      
      bool snapshot_updated = false;
      
      now = nowMS();
      if(init_barometer && now - last_barometer_ms >= BAROMETER_PERIOD_MS){
        snapshot.barometer = barometer.getData();
        last_barometer_ms = nowMS();
        have_barometer = snapshot.barometer.header.health == State::Health::HEALTHY;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(init_power_monitor && now - last_power_monitor_ms >= POWER_MONITOR_PERIOD_MS){
        snapshot.power_monitor = power_monitor.getData();
        last_power_monitor_ms = nowMS();
        have_power_monitor = snapshot.power_monitor.header.health == State::Health::HEALTHY;
        snapshot_updated = true;
      }
      
      now = nowMS();
      if(init_imu && now - last_imu_ms >= IMU_PERIOD_MS){
        snapshot.imu = imu.getData();
        last_imu_ms = nowMS();
        have_imu = snapshot.imu.header.health == State::Health::HEALTHY;
        snapshot_updated = true;
      }
      
      //ir lid
      //ir pq
      //cam?
      
      if(snapshot_updated){
        snapshot.captured_at_ms = nowMS();
          
        snapshot.summary = (static_cast<uint8_t>(snapshot.barometer.header.health) << 5) |
                           (static_cast<uint8_t>(snapshot.power_monitor.header.health) << 4) |
                           (static_cast<uint8_t>(snapshot.imu.header.health) << 3);
                           //(static_cast<uint8_t>(snapshot.ir_lid.header.health) << 2) |
                           //(static_cast<uint8_t>(snapshot.ir_pq.header.health) << 1) |
                           //static_cast<uint8_t>(cam.header.health)
        
        //publish snapshot
        if(xQueueOverwrite(runtime->sensor_snapshot_queue, &snapshot) == pdPASS)
          runtime->last_sensor_snapshot_published_ms.store(snapshot.captured_at_ms);
      }
      
      //time after a cycle of sensor task
      runtime->sensor_heartbeat_ms.store(nowMS()); 
      vTaskDelay(1);
    }
  }
}//namespace App
