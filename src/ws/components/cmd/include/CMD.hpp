#pragma once

#include <cstdint>
#include <cstring>

#include "States.hpp"
#include "Nodes.hpp"
#include "CRC.hpp"
#include "Msg.hpp"


namespace CMD{
  enum class Verb: uint8_t{
    START = 0,
    STOP = 1,
    KILL = 2,
    
    REBOOT = 5,
    
    CALIBRATE = 100,
    
    ENABLE = 200,
    ACTIVATE = 201,
  };
  
  
  enum class Noun: uint8_t{
    NODE = 0,
    MCU = 1,
    
    ALTITUDE = 100,
    IMU = 101,
    TIME = 102,
    
    FLIGHTMODE = 200,
    MECHANISM = 250,
  };
  
  //FreeRTOS ID, different from Node::ID
  enum class TaskID: uint32_t{
    SENSORS = 50,
    TELEMETRY = 60
  };
  
  
  struct __attribute__((__packed__)) CMD{//12 bytes
    Node::Address rx_addr;	//2 bytes	final destination of cmd
    				//		packet.header.receiver will have immediate destination(cmd parser etc)
    uint16_t sequence_number;	//2 bytes
    
    Verb verb;			//1 byte
    Noun noun;			//1 byte
    uint32_t param;		//4 bytes
    
    uint16_t crc;		//2 bytes
  };
  
  
  struct __attribute__((__packed__)) Ack{//8bytes
    uint16_t sequence_number;	//2 bytes
    Verb verb;			//1 byte
    Noun noun;			//1 byte
    uint32_t param;		//4 bytes
  };
  
  
  struct __attribute__((__packed__)) SCI{//4bytes
    uint16_t sequence_number;	//2 bytes
    uint16_t param;		//2 bytes
  };
  
  enum class RxResult: uint8_t{
    ACCEPTED = 0,
    
    NO_PACKET = 1,
    INVALID_CRC = 2,
    INVALID_CMD = 3,
  };
  
  CMD buildCMD(Node::Address rx_addr, uint16_t sq_num, 
               Verb verb, Noun noun, uint32_t param);
  
  bool validateCMD(const CMD& cmd);
  
  RxResult rxCMD(const Msg::Node& my_node, CMD& cmd);
  
  static_assert(sizeof(Node::Address) == 2);
  static_assert(sizeof(CMD) == 12);
  static_assert(sizeof(Ack) == 8);
  static_assert(sizeof(SCI) == 4);
  
  
}
    
