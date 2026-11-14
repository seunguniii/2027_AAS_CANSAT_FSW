#include "app_runtime.hpp"

#include <iostream>

#include "driver/gpio.h"

#include "I2C.hpp"
#include "MsgESPNOW.hpp"
#include "Sensors.hpp"
#include "States.hpp"
#include "MCU.hpp"

namespace{
  constexpr char TAG[] = "[Supervisor]";
    constexpr MCU::ID CTR_MCU = MCU::ID::ESP1;
    constexpr MCU::ID GS_MCU = MCU::ID::ESP_CAM_0;
    const Msg::Node super_node{Node::Address{CTR_MCU, Node::ID::SUPER}};
    const Msg::Node gs_telemetry_node{Node::Address{GS_MCU, Node::ID::TELEMETRY}};
  
  constexpr int64_t INIT_RETRY_INTERVAL_MS = 1000;
    
  //task heartbeat
  constexpr int64_t SENSOR_TASK_TIMEOUT_MS = 500;
  constexpr int64_t TELEMETRY_TASK_TIMEOUT_MS = 500;
  
  //i2c configs
  constexpr gpio_num_t SDA_PIN = GPIO_NUM_17;
  constexpr gpio_num_t SCL_PIN = GPIO_NUM_18;
  constexpr i2c_port_t I2C_PORT = I2C_NUM_0;
}


namespace Monitor{
  State::Health reportHealth(const char* name, bool stale_now, bool& previously_stale){
    //checks if 'name' is stale or recovered
    if (stale_now != previously_stale) {
      if(stale_now) std::cout << TAG << " " << name << " is stale." << std::endl;
      else std::cout << TAG << " " << name << " recovered." << std::endl;
    }
    previously_stale = stale_now;

    return stale_now? State::Health::FAULT : State::Health::HEALTHY;
  }
  
  uint8_t buildSummary(State::Health super, State::Health sensor, State::Health telemetry,
                       State::Health cmd_relay, State::Health actuator, State::Health logger){      
    return (static_cast<uint8_t>(super) << 5) | (static_cast<uint8_t>(sensor) << 4) | 
           (static_cast<uint8_t>(telemetry) << 3) | (static_cast<uint8_t>(cmd_relay) << 2) | 
           (static_cast<uint8_t>(actuator) << 1) | static_cast<uint8_t>(logger);
  }
  
}//namespace monitor


namespace INIT{
  bool runtimeResource(App::RuntimeContext* runtime){
    runtime->super_snapshot_queue = xQueueCreate(1, sizeof(App::SuperSnapshot));
    runtime->sensor_snapshot_queue = xQueueCreate(1, sizeof(App::SensorSnapshot));
    runtime->cmd_snapshot_queue = xQueueCreate(1, sizeof(App::CmdSnapshot));
  
    //delete all created resources in case of failure
    if(runtime->sensor_snapshot_queue == nullptr || 
       runtime->super_snapshot_queue == nullptr ||
       runtime->cmd_snapshot_queue == nullptr){
      std::cout << TAG << " Failed to create runtime resources." << std::endl;
      if(runtime->super_snapshot_queue != nullptr){
        vQueueDelete(runtime->super_snapshot_queue);
        runtime->super_snapshot_queue = nullptr;
      }if(runtime->sensor_snapshot_queue != nullptr){
        vQueueDelete(runtime->sensor_snapshot_queue);
        runtime->sensor_snapshot_queue = nullptr;
      }if(runtime->cmd_snapshot_queue != nullptr){
        vQueueDelete(runtime->cmd_snapshot_queue);
        runtime->cmd_snapshot_queue = nullptr;
      }
      std::cout << TAG << " Deleted all existing runtime resources." << std::endl;
      return false;
    }
    std::cout << TAG << " Runtime resources created." << std::endl; 
    //initial supervisor snapshot
    App::SuperSnapshot super{};
    super.captured_at_ms = App::nowMS();
    super.task_summary = Monitor::buildSummary(State::Health::HEALTHY, State::Health::FAULT,
                                               State::Health::FAULT,State::Health::FAULT,
                                               State::Health::FAULT, State::Health::FAULT);
    xQueueOverwrite(runtime->super_snapshot_queue, &super);

    //initial sensor snapshot
    App::SensorSnapshot sensor{};
    sensor.barometer.header.health = State::Health::FAULT;
    sensor.power_monitor.header.health = State::Health::FAULT;
    sensor.imu.header.health = State::Health::FAULT;
    sensor.captured_at_ms = App::nowMS();
    xQueueOverwrite(runtime->sensor_snapshot_queue, &sensor);

    //initial command snapshot
    App::CmdSnapshot cmd{};
    cmd.rx_result = CMD::RxResult::NO_PACKET;
    cmd.captured_at_ms = App::nowMS();
    xQueueOverwrite(runtime->cmd_snapshot_queue, &cmd);
    
    return true;
  }
  
  bool ESPNOW(){
    if(!Msg::detail::beginESPNOW()){
      std::cout << TAG << " ESP-NOW initialization failed." << std::endl;
      return false;
    }std::cout << TAG << " ESP-NOW initialized." << std::endl; return true;
  }//esp-now
  
  namespace Interface{
    bool I2C(App::RuntimeContext* runtime){
      I2C::BusConfig i2c_config{.port = I2C_PORT, .sda_pin = SDA_PIN,
                                .scl_pin = SCL_PIN, .internal_pullup = true};
      esp_err_t err = runtime->i2c_bus.init(i2c_config);

      if(err != ESP_OK){
        std::cout << TAG << " I2C initialization failed: " << esp_err_to_name(err) << std::endl;
        return false;
      }std::cout << TAG << " I2C initialized." << std::endl; return true;
    }//i2c
  }///namespace interface  
  
  namespace Task{
    bool sensor(App::RuntimeContext* runtime){
      if(xTaskCreate(App::sensorTask, "sensor_task", 6144, runtime, 5, &App::sensor_handle) != pdPASS){
        std::cout << TAG << " Failed to create SensorTask." << std::endl;
        vTaskDelete(App::sensor_handle);
        return false;
      }
      std::cout << TAG << " Successfully created SensorTask." << std::endl;
      return true;
    }
  
    bool telemetry(App::RuntimeContext* runtime){
      if(xTaskCreate(App::telemetryTask, "telemetry_task", 6144, runtime, 5, &App::telemetry_handle) != pdPASS){
        std::cout << TAG << " Failed to create TelemetryTask." << std::endl;
        vTaskDelete(App::telemetry_handle);
        return false;
      }
      std::cout << TAG << " Successfully created TelemetryTask." << std::endl;
      return true;
    }
  
    bool cmd_relay(App::RuntimeContext& runtime){return true;} //TODO
    bool actuator(App::RuntimeContext& runtime){return true;} //TODO
    bool logger(App::RuntimeContext& runtime){return true;} //TODO
  }//namespace Task
  
} //namespace INIT


namespace App{
  void supervisorTask(void* parameter){
    std::cout << TAG << " SupervisorTask started." << std::endl;
    
    const int64_t start_time = nowMS();
    
    SuperSnapshot snapshot{};
    
    bool init_runtime_resource = false;
    bool init_esp_now = false;
    bool init_i2c = false;
    
    bool init_sensor_task = false;
    bool init_telemetry_task = false;
    bool init_cmd_relay_task = false;
    bool init_actuator_task = false;
    bool init_logger_task = false;

    int64_t next_runtime_resource_init_ms = 0;
    int64_t next_esp_now_init_ms = 0;
    int64_t next_i2c_init_ms = 0;
    int64_t next_sensor_task_init_ms = 0;
    int64_t next_telemetry_task_init_ms = 0;
    int64_t next_cmd_relay_init_ms = 0;
    int64_t next_actuator_task_init_ms = 0;
    int64_t next_logger_task_init_ms = 0;
    
    bool sensor_task_stale = false;
    bool telemetry_task_stale = false;
    bool cmd_relay_task_stale = false;
    bool actuator_task_stale = false;
    bool logger_task_stale = false;
    
    State::Health supervisor_health = State::Health::HEALTHY;
    State::Health sensor_health = State::Health::FAULT;
    State::Health telemetry_health = State::Health::FAULT;
    State::Health cmd_relay_health = State::Health::FAULT;
    State::Health actuator_health = State::Health::FAULT;
    State::Health logger_health = State::Health::FAULT;
    
    bool task_snapshot_stale = false;
    
    //runtime resource has to be created before any task runs
    auto* runtime = static_cast<RuntimeContext*>(parameter);
    while(!init_runtime_resource){
      int64_t now = nowMS();
      if(now >= next_runtime_resource_init_ms){
        init_runtime_resource = INIT::runtimeResource(runtime);
        if (!init_runtime_resource) next_runtime_resource_init_ms = now + INIT_RETRY_INTERVAL_MS;
      }
      vTaskDelay(2);
    }
    runtime->started_at_ms.store(start_time);
    runtime->sensor_heartbeat_ms.store(start_time);
    runtime->telemetry_heartbeat_ms.store(start_time);
    
    
    bool show_worker_msg = false;
    while(true){
      const int64_t started_at = runtime->started_at_ms.load();  
      
      const int64_t telemetry_heartbeat = runtime->telemetry_heartbeat_ms.load();
      int64_t now = nowMS();
      if(!init_esp_now && now >= next_esp_now_init_ms){
        init_esp_now = INIT::ESPNOW();
        if (!init_esp_now) next_esp_now_init_ms = now + INIT_RETRY_INTERVAL_MS;
      }else{
        if(!init_telemetry_task && now >= next_telemetry_task_init_ms){
          init_telemetry_task = INIT::Task::telemetry(runtime);
          if(!init_telemetry_task) next_telemetry_task_init_ms = now + INIT_RETRY_INTERVAL_MS;
        }else{
          telemetry_health = Monitor::reportHealth("TelemetryTask",
                             telemetry_heartbeat == 0 || now - telemetry_heartbeat > TELEMETRY_TASK_TIMEOUT_MS,
                             telemetry_task_stale);
        }
        //TODO if(telemetry_health == faulty)
        //TODO   my_node.send(gcs_node, lightweight telemetry including task health and cmd ack);
        //TODO super cmd rx
        //TODO cmd relay task
      }
        
      const int64_t sensor_heartbeat = runtime->sensor_heartbeat_ms.load();
      now = nowMS();
      if(!init_i2c && now >= next_i2c_init_ms){ //add interfaces if needed
          init_i2c = INIT::Interface::I2C(runtime);
          if (!init_i2c) next_i2c_init_ms = now + INIT_RETRY_INTERVAL_MS;
      }else{
        if(!init_sensor_task && now >= next_sensor_task_init_ms){
          init_sensor_task = INIT::Task::sensor(runtime);
          if (!init_sensor_task) next_sensor_task_init_ms = now + INIT_RETRY_INTERVAL_MS;
        }else{
          sensor_health = Monitor::reportHealth("SensorTask",
                          sensor_heartbeat == 0 || now - sensor_heartbeat > SENSOR_TASK_TIMEOUT_MS,
                          sensor_task_stale);
        }
      }
      
      //TODO actuator task
      //TODO logger task
      
      bool workers_released = false;
      if (!workers_released &&
          init_sensor_task && init_telemetry_task) {
        runtime->started_at_ms.store(nowMS());

        workers_released = true;
        if(!show_worker_msg){
          std::cout << TAG << " Startup gate released. Workers may begin." << std::endl;
          show_worker_msg = true;
        }
      }
      
      //TODO task handling
      
      const int64_t last_snapshot = runtime->last_super_snapshot_published_ms.load();
      const int64_t last_tx_accepted = runtime->last_tx_request_accepted_ms.load();
      
      snapshot.task_summary = Monitor::buildSummary(supervisor_health, sensor_health,
                                                    telemetry_health, cmd_relay_health,
                                                    actuator_health, logger_health);
      snapshot.captured_at_ms = nowMS();
      xQueueOverwrite(runtime->super_snapshot_queue, &snapshot);
      runtime->last_super_snapshot_published_ms.store(snapshot.captured_at_ms);
      
      vTaskDelay(2); //2 ticks
    }
  }
  
}//namespace App
