#include "driver/gpio.h"

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "I2C.hpp"
#include "MsgESPNOW.hpp"
#include "Sensors.hpp"

#include "app_runtime.hpp"

namespace{
  constexpr char TAG[] = "[CTR]";
  
  //i2c configs
  constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
  constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
  constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
}

extern "C" void app_main(void)
{
  std::cout << TAG << " Initializing application." << std::endl;
  
  //initialize i2c
  static I2C::Bus i2c_bus;
  I2C::BusConfig i2c_config{
    .port = I2C_PORT,
    .sda_pin = SDA_PIN,
    .scl_pin = SCL_PIN,
    .internal_pullup = true
  };
  
  esp_err_t err = i2c_bus.init(i2c_config);
  
  if(err != ESP_OK){
    std::cout << TAG << " I2C initialization failed: " << esp_err_to_name(err) << std::endl;
    return;
  }
  std::cout << TAG << " I2C initialized." << std::endl;
  
  
  //initialize sensors
  if(!Barometer::init(i2c_bus)){
    std::cout << TAG << " BMP581 initialization failed." << std::endl;
    return;
  }
  std::cout << TAG << " BMP581 initialized." << std::endl;
  
  
  //intialize esp-now
  if(!Msg::detail::beginESPNOW()){
  std::cout << TAG << " ESP-NOW initialization failed." << std::endl;
    return;
  }
  std::cout << TAG << " ESP-NOW initialized." << std::endl;
  
  
  //craete resources
  static App::RuntimeContext runtime{};
  
  runtime.sensor_snapshot_queue = xQueueCreate(1, sizeof(App::SensorSnapshot));
  runtime.start_gate = xEventGroupCreate();
  
  if(runtime.sensor_snapshot_queue == nullptr || runtime.start_gate == nullptr){
    std::cout << TAG << " Failed to create runtime resources." << std::endl;
    
    if(runtime.sensor_snapshot_queue != nullptr){
      vQueueDelete(runtime.sensor_snapshot_queue);
      runtime.sensor_snapshot_queue = nullptr;
    }
    
    if(runtime.start_gate != nullptr){
      vEventGroupDelete(runtime.start_gate);
      runtime.start_gate = nullptr;
    }
    return;
  }
  const int64_t start_time = App::nowMS();
  runtime.started_at_ms.store(start_time);
  runtime.sensor_heartbeat_ms.store(start_time);
  runtime.telemetry_heartbeat_ms.store(start_time);
  
  
  //create workers
  TaskHandle_t sensor_handle = nullptr;
  TaskHandle_t telemetry_handle = nullptr;
  TaskHandle_t supervisor_handle = nullptr;
  
  auto cleanup_unstarted_tasks = [&](){
    if(supervisor_handle != nullptr) vTaskDelete(supervisor_handle);
    if(telemetry_handle != nullptr) vTaskDelete(telemetry_handle);
    if(sensor_handle != nullptr) vTaskDelete(sensor_handle);
    
    vEventGroupDelete(runtime.start_gate);
    vQueueDelete(runtime.sensor_snapshot_queue);
    
    runtime.start_gate = nullptr;
    runtime.sensor_snapshot_queue = nullptr;
  };
  
  if(xTaskCreate(App::sensorTask, "sensor_task", 6144, &runtime, 5, &sensor_handle) != pdPASS){
    std::cout << TAG << " Failed to create SensorTask." << std::endl;
    cleanup_unstarted_tasks();
    return;
  }
  
  if(xTaskCreate(App::telemetryTask, "telemetry_task", 6144, &runtime, 5, &telemetry_handle) != pdPASS){
    std::cout << TAG << " Failed to create TelemetryTask." << std::endl;
    cleanup_unstarted_tasks();
    return;
  }
  
  
  if(xTaskCreate(App::supervisorTask, "supervisor_task", 4096, &runtime, 3, &supervisor_handle) != pdPASS){
    std::cout << TAG << " Failed to create SupervisorTask." << std::endl;
    cleanup_unstarted_tasks();
    return;
  }
  
  
  //*whips workers to get to work*
  xEventGroupSetBits(runtime.start_gate, App::TASKS_START_BIT);
  std::cout << TAG << " All application tasks started." << std::endl;
  
  return; 
}
