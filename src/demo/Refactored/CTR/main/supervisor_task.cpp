#include "app_runtime.hpp"

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace App{
  namespace{
    constexpr char TAG[] = "[Supervisor]";
    
    //2s startup grace
    //avoid error before full worker startup
    constexpr int64_t STARTUP_GRACE_MS = 2000;
    
    //task heartbeat
    constexpr int64_t SENSOR_TASK_TIMEOUT_MS = 500;
    constexpr int64_t TELEMETRY_TASK_TIMEOUT_MS = 500;
    
    //actual process time period
    constexpr int64_t SENSOR_SNAPSHOT_TIMEOUT_MS = 500;
    constexpr int64_t TELEMETRY_SEND_TIMEOUT_MS = 1000;
    
    void reportTransition(const char* name, bool stale_now, bool& previously_stale){
      /*checks if 'name' is stale or recovered*/
      if(stale_now && !previously_stale) std::cout << TAG << " " << name << " is stale." << std::endl;
      else if(!stale_now && previously_stale) std::cout << TAG << " " << name << " recovered." << std::endl;
      
      previously_stale = stale_now;
    }
  }//namepsace
  
  
  void supervisorTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    xEventGroupWaitBits(runtime->start_gate, TASKS_START_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
    std::cout << TAG << " SupervisorTask started." << std::endl;
    
    bool sensor_task_stale = false;
    bool telemetry_task_stale = false;
    bool snapshot_stale = false;
    bool telemetry_send_stale = false;
    
    while(true){
      const int64_t now = nowMS();
      const int64_t started_at = runtime->started_at_ms.load();
      
      if(now - started_at >= STARTUP_GRACE_MS){
        const int64_t sensor_heartbeat = runtime->sensor_heartbeat_ms.load();
        const int64_t telemetry_heartbeat = runtime->telemetry_heartbeat_ms.load();
        const int64_t last_snapshot = runtime->last_snapshot_published_ms.load();
        const int64_t last_tx_accepted = runtime->last_tx_request_accepted_ms.load();
        
        reportTransition("SensorTask",
                         sensor_heartbeat == 0 || now - sensor_heartbeat > SENSOR_TASK_TIMEOUT_MS,
                         sensor_task_stale);
                         
        reportTransition("TelemetryTask",
                         telemetry_heartbeat == 0 || now - telemetry_heartbeat > TELEMETRY_TASK_TIMEOUT_MS,
                         telemetry_task_stale);

        reportTransition("Sensor snapshots",
                         last_snapshot == 0 || now - last_snapshot > TELEMETRY_SEND_TIMEOUT_MS,
                         telemetry_send_stale);
      }
      vTaskDelay(2);
    }
  }//supervisorTask
  
}//namespace App
