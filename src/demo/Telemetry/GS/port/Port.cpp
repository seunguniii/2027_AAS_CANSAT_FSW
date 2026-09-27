#include <cstdint>
#include <cstddef>
#include<cstring>

#include "freertos/FreeRTOS.h"
#include "driver/usb_serial_jtag.h"

#include "CRC.h"
#include "Port.h"

namespace Port{
  namespace{
    constexpr uint8_t SYNC_0 = 0xAA;
    constexpr uint8_t SYNC_1 = 0x55;
    
    constexpr size_t SYNC_SIZE = 2;
    constexpr size_t FRAME_TYPE_SIZE = 1;
    constexpr size_t CRC_SIZE = sizeof(uint16_t);
    
    constexpr size_t FRAME_SIZE = SYNC_SIZE + FRAME_TYPE_SIZE + sizeof(Msg::Packet) + CRC_SIZE;
    
    constexpr TickType_t IO_TIMEOUT = pdMS_TO_TICKS(100);
    
    uint8_t rx_frame[FRAME_SIZE]{};
    size_t rx_count = 0;
    
    bool validFrameType(uint8_t value){
      switch(static_cast<FrameType>(value)) {
        case FrameType::CTR_TELEMETRY:
        case FrameType::PQ_TELEMETRY:
        case FrameType::CMD:
          return true;
          
        default:
          return false;
      }
    }
      
    uint16_t readCRC(const uint8_t* data) {
      uint16_t crc{};
        
      std::memcpy(&crc, data, sizeof(crc));
      return crc;
    }
      
    void resetReceiver(){
      rx_count = 0;
    }
      
    bool processFrame(FrameType& type, Msg::Packet& packet) {
      //frame structure
      //[0]			SYNC_0
      //[1]			SYNC_1
      //[2]			FrameType
      //[3 - last-2]		Msg::Packet
      //[last-1 - last]	CRC
        
      uint8_t frame_type_raw = rx_frame[2];
        
      if(!validFrameType(frame_type_raw)){
        resetReceiver();
        return false;
      }
        
      const size_t crc_offset = SYNC_SIZE + FRAME_TYPE_SIZE + sizeof(Msg::Packet);
      uint16_t received_crc = readCRC(&rx_frame[crc_offset]);
      uint16_t calculated_crc = utils::calculateCRC16(
        &rx_frame[2], FRAME_TYPE_SIZE + sizeof(Msg::Packet)
      );
        
      if(received_crc != calculated_crc){
        resetReceiver();
        return false;
      }
      
      resetReceiver();
      return true;
    }
  }//namespace
    
    
  bool begin(void) {
    if(usb_serial_jtag_is_driver_installed()) return true;
      
    usb_serial_jtag_driver_config_t config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
      
    return usb_serial_jtag_driver_install(&config) == ESP_OK;
  }
    
    
  bool sendPacket(FrameType type, const Msg::Packet& packet){
    uint8_t frame[FRAME_SIZE]{};
    
    frame[0] = SYNC_0; frame[1] = SYNC_1;
    frame[2] = static_cast<uint8_t>(type);
    
    std::memcpy(&frame[3], &packet, sizeof(Msg::Packet));
      
    const size_t crc_offset = SYNC_SIZE + FRAME_TYPE_SIZE + sizeof(Msg::Packet);
      
    uint16_t crc = utils::calculateCRC16(
      &frame[2], FRAME_TYPE_SIZE + sizeof(Msg::Packet)
    );
    std::memcpy(&frame[crc_offset], &crc, sizeof(crc));
     
    int written = usb_serial_jtag_write_bytes(frame, sizeof(frame), IO_TIMEOUT);
      
    return written == static_cast<int>(sizeof(frame));
  }
        
        
  bool getPacket(FrameType& type, Msg::Packet& packet){
    uint8_t buffer[64]{};
      
    int bytes_read = usb_serial_jtag_read_bytes(buffer, sizeof(buffer), 0);
      
    if(bytes_read <= 0) return false;
      
    for(int i = 0; i < bytes_read; i++){
      uint8_t byte = buffer[i];
        
      //find sync bytes
      if(rx_count == 0){
        if(byte == SYNC_0) rx_frame[rx_count++] = byte;
        
        continue;
      }
      
      if(rx_count == 1){
        if(byte == SYNC_1) rx_frame[rx_count++] = byte;
        
        else if(byte == SYNC_0){
          rx_frame[0] = SYNC_0;
          rx_count = 1;
        } else resetReceiver();
        
        continue;
      }
      //found sync bytes
      
      rx_frame[rx_count++] = byte;
      
      if(rx_count == FRAME_SIZE){
        if(processFrame(type, packet)) return true;
      }
    }
    
    return false;
  }
}//namespace port
