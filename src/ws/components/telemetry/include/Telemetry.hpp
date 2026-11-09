#pragma once

#include <cstdint>
#include <cstring>

#include "States.hpp"
#include "CMD.hpp"
#include "CRC.hpp"
#include "Sensors.hpp"
namespace Msg{class Node;} //avoid circular dependency

namespace Telemetry {
  struct __attribute__((__packed__)) CTR{
    const char ID[6] = "4047C";	//						6bytes
    uint32_t mission_time;	//	HHMMSSmmm				4bytes
    uint16_t packet_count;	//	telemetry packet counter		2bytes
    uint16_t command_count;	//	command received counter		2bytes
  
    //states
    FlightMode mode;		//flight mode, flight/sim			1byte
    CTR_OpState state;		//container state				1byte
    uint8_t mech_state;		//hex, bit0 dply mech, bit1 pq released		1byte
    uint16_t task_state;	//bitmaske	subtask state, 2bits each	2bytes (14bit)
  
    //sensors
    float altitude;		//[m]						4bytes
    float pressure;		//[pa]						4bytes
    float temperature;		//[C*]						4bytes
    float battery_voltage;	//[V]						4bytes
    float battery_current;	//[ma]						4bytes
    float rot_rate[3];		//[deg/s]	roll, pitch, yaw		12bytes
    float acc[3];		//[m/s^2]	east, south, down		12bytes
    float mag[3];		//[mG]		east, south, down		12bytes
    uint16_t sensor_state;	//bitmask	sensor+cam status, 2bits each	2bytes (12bits)
  
    //cmd [uint16_t sequence_num] [uint8_t verb] [uint8_t noun] [uint32_t param]
    CMD::Ack cmd_rx;		//last received cmd				8bytes
    CMD::RxResult rx_result;	//how did receiving/parsing of the cmd go?	1byte
    CMD::Ack cmd_exec;		//last executed cmd				8bytes
    
    //reserved 									33bytes
    
    uint16_t crc;		//crc16-citt checksum				2bytes
  };//struct CTR_Telemetry							95bytes + reserved 33bytes
  
  
  struct __attribute__((__packed__)) PQ{
    const char ID[6] = "4047P"; //						6bytes
    uint32_t mission_time;	//[ms]	time since boot				4bytes
    uint16_t packet_count;	//	telemetry packet counter		2bytes
    uint16_t command_count;	//	command received counter		2bytes
  
    //states
    FlightMode mode;		//flight mode, flight/sim			1byte
    uint8_t mech_state;		//hex, bit0 sp1 dply, bit1 sp2 dply, 		1byte
				//     bit2 mgnt dply, bit3 cam rec
    uint16_t task_state;	//bitmaske	subtask state, 2bits each	2bytes (14bits)
  
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
    
    uint32_t sensor_state;	//bitmask	sensor+cam status, 2bits each	4bytes (22bits)
  
    //cmd [uint16_t sequence_num] [uint8_t verb] [uint8_t noun] [uint32_t param]
    CMD::Ack cmd_rx;		//last received cmd				8bytes
    CMD::RxResult cmd_rx_result;//how did receiving/parsing of the cmd go?	1byte
    CMD::Ack cmd_exec;		//last executed cmd				8bytes
    CMD::SCI cmd_sci;		//science experiment				4bytes
    CMD::RxResult sci_rx_result;//how did receiving/parsing of cmd::sci go?	1byte
    
    //reserved									1byte
    
    uint16_t crc;			//crc16-citt checksum			2bytes
  };//struct PQ_Telemetry							127bytes + reserved 1byte
  

  //helpers for crc (packet verification)  
  template<typename T>
  inline void buildCRC(T& packet){
    packet.crc = utils::calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
  }
  
  template<typename T>
  inline bool verifyCRC(const T& packet){
    uint16_t computed = utils::calculateCRC16(
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(T) - sizeof(uint16_t)
    );
    return (packet.crc == computed);
  }
  
  bool sendCTR(const Msg::Node& this_node, const Msg::Node& peer_node,
               const Sensor::Barometer& barometer, 
               const Sensor::PowerMonitor& power_monitor,
               FlightMode mode, CTR_OpState state, uint8_t mech_state);
               
  bool sendPQ(const Msg::Node& this_node, const Msg::Node& peer_node,
              const Sensor::Barometer& barometer, 
              const Sensor::PowerMonitor& power_monitor,
              const Sensor::IMU& imu,
              const Sensor::Magnetometer& magnetometer,
              const Sensor::GNSS& gnss,
              const Sensor::PowerMonitor& sp1_monitor,
              const Sensor::PowerMonitor& sp2_monitor,
              FlightMode mode, uint8_t mech_state);
}//namespace telemetry
