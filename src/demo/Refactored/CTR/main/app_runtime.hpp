#pragma once

#include <atomic>
#include <cstdint>

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "Sensors.hpp"

namespace App{
  //all workers wait until app_main has created them
  constexpr EventBits_t TASKS_START_BIT = (1U << 0);
  
  inline int64_t nowMS(){return esp_timer_get_time() / 1000LL;}
  
  //latest sensor readings
  struct SensorSnapshot{
    //TODO add sensors
    Sensor::Barometer barometer{};
    Sensor::PowerMonitor power_monitor{};
    int64_t captured_at_ms = 0;
  };
  
  //system status
  struct RuntimeContext{
    QueueHandle_t sensor_snapshot_queue = nullptr;
    EventGroupHandle_t start_gate = nullptr;
    
    std::atomic<int64_t> started_at_ms{0};
    
    std::atomic<int64_t> sensor_heartbeat_ms{0};
    std::atomic<int64_t> telemetry_heartbeat_ms{0};
    
    std::atomic<int64_t> last_snapshot_published_ms{0};
    std::atomic<int64_t> last_tx_request_accepted_ms{0};
    
    std::atomic<int64_t> telemetry_send_failures{0};
  };
  
  //add tasks
  void sensorTask(void* parameter);
  void telemetryTask(void* parameter);
  void supervisorTask(void* parameter);
  
}//namespace app
