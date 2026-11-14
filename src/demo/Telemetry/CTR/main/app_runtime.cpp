#include "app_runtime.hpp"

namespace App {
  TaskHandle_t supervisor_handle = nullptr;
  TaskHandle_t sensor_handle = nullptr;
  TaskHandle_t telemetry_handle = nullptr;
  TaskHandle_t cmd_relay_handle = nullptr;
  TaskHandle_t actuator_handle = nullptr;
  TaskHandle_t logger_handle = nullptr;
  
  Barometer::BMP581 barometer{};
  PowerMonitor::INA228 power_monitor{};
  IMU::BNO085 imu{};
}
