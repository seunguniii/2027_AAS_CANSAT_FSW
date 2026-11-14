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
    
    constexpr MCU::ID CTR_MCU = MCU::ID::ESP1;
    constexpr MCU::ID GS_MCU = MCU::ID::ESP_CAM_0;
    
    const Msg::Node ctr_telemetry_node{Node::Address{CTR_MCU, Node::ID::TELEMETRY}};
    const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};
  }
  
  void telemetryTask(void* parameter){
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    
    std::cout << TAG << " TelemetryTask started." << std::endl;
    
    SuperSnapshot super_snapshot{};
    SensorSnapshot sensor_snapshot{};
    CmdSnapshot cmd_snapshot{};
    
    int64_t last_telemetry_ms = nowMS();
    
    while(true){
      int64_t now = nowMS();
      if(now - last_telemetry_ms >= TELEMETRY_PERIOD_MS){
        if(xQueuePeek(runtime->super_snapshot_queue, &super_snapshot, 0) == pdTRUE &&
           xQueuePeek(runtime->sensor_snapshot_queue, &sensor_snapshot, 0) == pdTRUE &&
           xQueuePeek(runtime->cmd_snapshot_queue, &cmd_snapshot, 0) == pdTRUE){
          uint8_t MECH_STATE = 0x00; //TODO build from super snapshot
          const bool sent = Telemetry::sendCTR(ctr_telemetry_node, gs_telemetry_node,
                                                   sensor_snapshot.barometer, sensor_snapshot.power_monitor,
                                                   sensor_snapshot.imu, sensor_snapshot.summary, 
                                                   super_snapshot.flight_mode, super_snapshot.op_state, 
                                                   MECH_STATE, super_snapshot.task_summary,
                                                   cmd_snapshot.cmd_rx, cmd_snapshot.rx_result, cmd_snapshot.cmd_exec);
          
          if(sent) runtime->last_tx_request_accepted_ms.store(nowMS()); //if telemetry tx success
          else{
            const uint32_t failures = runtime->telemetry_send_failures.fetch_add(1) + 1;
            if(failures == 1 || failures % 10 == 0)
              std::cout << TAG << " Telemetry send request failed. (" << static_cast<unsigned long>(failures) << " failures)" << std::endl;
          }
        }last_telemetry_ms = now;
      }
      runtime->telemetry_heartbeat_ms.store(nowMS());
      vTaskDelay(1);
    }//while
  }//telemetryTask()
  
}//namespace App
