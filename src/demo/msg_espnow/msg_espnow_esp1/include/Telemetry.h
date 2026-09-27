#pragma once

#include <cstdint>
#include <cstring>

#include "States.h"
#include "CMD.h"
#include "CRC.h"

namespace Telemetry {

  struct __attribute__((__packed__)) CTR{
    char ID[6];			//	Team number(4 digit int) + "C"\0	6bytes
    uint32_t mission_time;	//[ms]	time since boot				4bytes
    uint16_t packet_count;	//	telemetry packet counter		2bytes
    uint16_t command_count;	//	command received counter		2bytes
  
    //states
    FlightMode mode;		//flight mode, flight/sim			1byte
    CTR_OpState state;		//container state				1byte
    uint8_t mech_state;		//hex, bit0 dply mech, bit1 pq released		1byte
  
    //sensors
    float altitude;		//[m]						4bytes
    float pressure;		//[pa]						4bytes
    float temperature;		//[C*]						4bytes
    float battery_voltage;	//[V]						4bytes
    float battery_current;	//[ma]						4bytes
  
    //miscellaneous
    CMD::CTR cmd_echo;		//last received & processed cdm by ctr		1byte
    uint16_t crc;		//crc16-citt checksum				2byte
  };//struct CTR_Telemetry							40bytes
  
  
  struct __attribute__((__packed__)) PQ{
    char ID[6];			//	Team number(4 digit int) + "P"\0	6bytes
    uint32_t mission_time;	//[ms]	time since boot				4bytes
    uint16_t packet_count;	//	telemetry packet counter		2bytes
    uint16_t command_count;	//	command received counter		2bytes
  
    //states
    FlightMode mode;		//flight mode, flight/sim			1byte
    uint8_t mech_state;		//hex, bit0 sp1 dply, bit1 sp2 dply, 		1byte
				//     bit2 mgnt dply, bit3 cam rec
  
    //sensors
    float altitude;		//[m]						4bytes
    float pressure;		//[pa]						4bytes
    float temperature;		//[C*]						4bytes
    float voltage;		//[V]		battery voltage			4bytes
    float current;		//[ma]		battery current			4bytes
    float rot_rate[3];		//[deg/s]	roll, pitch, yaw		12bytes
    float acc[3];		//[m/s^2]	east, south, down		12bytes
    float mag[3];		//[mG]		east, south, down		12bytes
    
    uint32_t gnss_time;		//[def]	gnss time				4bytes
    float gnss_pos[3];		//[deg, deg, m]	lat lon alt			12bytes
    uint8_t gnss_sats;		//		# of sats			1byte
    
    float solar_1;		//[V]		sp voltage			4bytes
    float solar_2;		//[V]		sp voltage			4bytes
  
    //miscellaneous
    CMD::PQ cmd_echo;			//last received & processed cmd by pq	1byte
    CMD::PQ_IMG_STB image_stabilization;//image stabilization metric		1byte
    CMD::PQ_SCI_EXP science_exp;	//science experiment			1byte
    uint16_t crc;			//crc16-citt checksum			2bytes
  };//struct PQ_Telemetry							102bytes
  

  //helpers for crc (packet verification)  
  template<typename T>
  inline void buildCRC(T& packet){
    packet.crc = utils::calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
  }
  
  template<typename T>
  inline bool verifyCRC(T& packet){
    uint16_t computed = utils::calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
    return (packet.crc == computed);
  }

}//namespace msg
