#pragma once

#include <atomic>
#include <cstdint>

#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "Sensors.hpp"
#include "CMD.hpp"
#include "I2C.hpp"

namespace App{
  //all workers wait until app_main has created them
  constexpr EventBits_t TASKS_START_BIT = (1U << 0);
  
  inline int64_t nowMS(){return esp_timer_get_time() / 1000LL;}
  
  //TODO add sensors
  //sensors
  extern Barometer::BMP581 barometer;
  extern PowerMonitor::INA228 power_monitor;
  extern IMU::BNO085 imu;
  
  //snapshots
  struct SuperSnapshot{
    State::Flight flight_mode = State::Flight::FLIGHT;
    State::CTR_OP op_state;
    State::Mech lid_state;
    State::Mech pq_dply_state;
    
    uint8_t task_summary; //
    int64_t captured_at_ms = 0;
  };
  
  //latest sensor readings
  struct SensorSnapshot{
    Sensor::Barometer barometer{};
    Sensor::PowerMonitor power_monitor{};
    Sensor::IMU imu{};
    //IR LID
    //IR PQ
    uint8_t summary; //BAR | PM | IMU | IRLID | IRPQ | CAM
    int64_t captured_at_ms = 0;
  };
  
  struct CmdSnapshot{
    CMD::Ack cmd_rx{};
    CMD::RxResult rx_result;
    CMD::Ack cmd_exec{};
    int64_t captured_at_ms = 0;
  };
  
  //system logs
  struct RuntimeContext{
    I2C::Bus i2c_bus;
    
    QueueHandle_t super_snapshot_queue = nullptr;
    QueueHandle_t sensor_snapshot_queue = nullptr;
    QueueHandle_t cmd_snapshot_queue = nullptr;
    
    std::atomic<int64_t> started_at_ms{0};
    
    std::atomic<int64_t> sensor_heartbeat_ms{0};
    std::atomic<int64_t> telemetry_heartbeat_ms{0};
    std::atomic<int64_t> cmd_relay_heartbeat_ms{0};
    std::atomic<int64_t> actuator_heartbeat_ms{0};
    std::atomic<int64_t> logger_heartbeat_ms{0};
    
    std::atomic<int64_t> last_super_snapshot_published_ms{0};
    std::atomic<int64_t> last_sensor_snapshot_published_ms{0};
    std::atomic<int64_t> last_cmd_snapshot_published_ms{0};
    std::atomic<int64_t> last_tx_request_accepted_ms{0};
    
    std::atomic<int64_t> telemetry_send_failures{0};
  };
  
  //add tasks
  void sensorTask(void* parameter);
  void telemetryTask(void* parameter);
  void supervisorTask(void* parameter);
  void cmdRelayTask(void* parameter);
  void actuatorTask(void* parameter);
  void loggerTask(void* parameter);
  
  //task handles
  extern TaskHandle_t supervisor_handle;
  extern TaskHandle_t sensor_handle;
  extern TaskHandle_t telemetry_handle;
  extern TaskHandle_t cmd_relay_handle;
  extern TaskHandle_t actuator_handle;
  extern TaskHandle_t logger_handle;
}//namespace app
