#include <iostream>
#include <cstdio>

#include <cstring>

#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "MsgESPNOW.h"
#include "MCU.h"

namespace Msg::detail{
  namespace{
    constexpr uint8_t ESPNOW_CHANNEL = 1;
    constexpr size_t RX_QUEUE_SIZE = 16;
    
    QueueHandle_t rx_queue = nullptr;
    
    //check valid mac addr
    bool isZeroMac(const MCU::MacAddress& mac){
      for(uint8_t byte: mac){
        if(byte != 0) return false;
      }
      return true;
    }
    
    
    //add MCU
    bool ensurePeer(MCU::ID mcu){
      const auto& mac = MCU::macAddress(mcu);

      if(isZeroMac(mac)) return false;
      if(esp_now_is_peer_exist(mac.data())) return true;
      
      esp_now_peer_info_t peer{};
      std::memcpy(peer.peer_addr, mac.data(), ESP_NOW_ETH_ALEN);
      
      peer.channel = ESPNOW_CHANNEL;
      peer.ifidx = WIFI_IF_STA;
      peer.encrypt = false;
      
      esp_err_t err = esp_now_add_peer(&peer);
              
      if(err == ESP_OK || err == ESP_ERR_ESPNOW_EXIST) return true;
      
      return false;
    }
    
    //receive callback
    void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int data_len){
      (void)info;
      
      if(data == nullptr) return;
      if(data_len != sizeof(Packet)) return;
      
      Packet packet{};
      std::memcpy(&packet, data, sizeof(Packet));
      
      xQueueSend(rx_queue, &packet, 0);
    }
    
    void receiveTask(void*){
      Packet packet{};
      
      while(true){
        if(xQueueReceive(rx_queue, &packet, portMAX_DELAY) == pdTRUE) enqueuePacket(packet);
      }
    }
  }//namespace
  
  bool beginESPNOW(){
    esp_err_t err;
    
    err = esp_netif_init();
    std::cout << "esp_netif_init: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;
    
    err = esp_event_loop_create_default();
    std::cout << "esp_event_loop_create_default: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE) return false;
    
    err = nvs_flash_init();
    std::cout << "nvs_flash_init: " << esp_err_to_name(err) << "\n";
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
      err = nvs_flash_erase();
      if(err != ESP_OK) return false;
      err = nvs_flash_init();
    }
    if(err != ESP_OK) return false;
    
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifi_config);
    std::cout << "esp_wifi_init: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK && err != ESP_ERR_WIFI_INIT_STATE) return false;
    
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    std::cout << "esp_wifi_set_mode: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK) return false;
    
    err = esp_wifi_start();
    std::cout << "esp_wifi_start: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK && err != ESP_ERR_WIFI_STATE) return false;
    
    err = esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    std::cout << "esp_wifi_set_channel: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK) return false;
    
    err = esp_now_init();
    std::cout << "esp_now_init: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK) return false;
    
    rx_queue = xQueueCreate(RX_QUEUE_SIZE, sizeof(Packet));
    if(rx_queue == nullptr) return false;
    
    err = esp_now_register_recv_cb(onReceive);
    std::cout << "esp_now_register_recv_cb: " << esp_err_to_name(err) << "\n";
    if(err != ESP_OK) return false;
    
    BaseType_t task_result = xTaskCreate(
      receiveTask, "espnow_rx", 4096, nullptr, 5, nullptr
    );
    if(task_result != pdPASS) return false;
    
    return true;
  }
  
  bool sendESPNOWPacket(const Packet& packet){
    const auto& mac = MCU::macAddress(packet.header.receiver.mcu);
    
    if(isZeroMac(mac)) return false;
    if(!ensurePeer(packet.header.receiver.mcu)) return false;
    
    esp_err_t err = esp_now_send(
      mac.data(),
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(Packet)
    );
    
    return err == ESP_OK;
  }
}//namespace Msg::detail

    
