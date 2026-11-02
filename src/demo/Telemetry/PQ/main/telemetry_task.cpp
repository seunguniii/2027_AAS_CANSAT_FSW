#include "app_runtime.hpp"

#include <cstdint>

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "MCU.hpp"
#include "Msg.hpp"
#include "Telemetry.hpp"

namespace App{
  namespace{
    constexpr char TAG[] = "[TelemetryTask]";
    
    int64_t TELEMETRY_PERIOD_MS = 250; //TODO change wrt mission stage
    
    constexpr MCU::ID PQ_MCU = MCU::ID::ESP_CAM_1;
    constexpr MCU::ID GS_MCU = MCU::ID::ESP_CAM_0;
    
    const Msg::Node pq_telemetry_node{Node::Address{PQ_MCU, Node::ID::TELEMETRY}};
    const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};
    
    FlightMode MODE = FlightMode::SIMULATION; //TODO set with cmd
    uint8_t MECH_STATE = 0x00;
  }
  
  void telemetryTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    //wait until all workers are created
    xEventGroupWaitBits(runtime->start_gate, TASKS_START_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
    
    std::cout << TAG << " TelemetryTask started." << std::endl;
    
    SensorSnapshot snapshot{};
    
    int64_t last_telemetry_ms = nowMS();
    
    while(true){
      int64_t now = nowMS();
      if(now - last_telemetry_ms >= TELEMETRY_PERIOD_MS){
        if(xQueuePeek(runtime->sensor_snapshot_queue, &snapshot, 0) == pdTRUE){
          const bool accepted = Telemetry::sendPQ(pq_telemetry_node, gs_telemetry_node,
                                                   snapshot.barometer, snapshot.power_monitor,
                                                   snapshot.imu, snapshot.magnetometer,
                                                   snapshot.gnss, snapshot.sp0, snapshot.sp1,
                                                   MODE, MECH_STATE);
          
          if(accepted) runtime->last_tx_request_accepted_ms.store(nowMS()); //if telemetry tx success
          else{
            const uint32_t failures = runtime->telemetry_send_failures.fetch_add(1) + 1;
            if(failures == 1 || failures % 10 == 0)
              std::cout << TAG << " Telemetry send request failed. (" << static_cast<unsigned long>(failures) << " failures)" << std::endl;
          }
        }
        last_telemetry_ms = now;
      }
      runtime->telemetry_heartbeat_ms.store(nowMS());
      vTaskDelay(1);
    }//while
  }//telemetryTask()
  
}//namespace App
