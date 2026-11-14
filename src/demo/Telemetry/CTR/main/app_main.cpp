#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "app_runtime.hpp"


extern "C" void app_main(void)
{
  std::cout << "[CTR] Application booting." << std::endl;
  static App::RuntimeContext runtime{};

  if (xTaskCreate(App::supervisorTask, "supervisor_task",
                  6144, &runtime, 3, &App::supervisor_handle) != pdPASS) {
    std::cout << "[CTR] Failed to create SupervisorTask." << std::endl;
    esp_restart();
  }
}
