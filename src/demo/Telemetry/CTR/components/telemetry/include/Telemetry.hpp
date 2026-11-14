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
    const char ID[6] = "4047C";	//						6bytes	0
    uint32_t mission_time;	//	HHMMSSmmm				4bytes	6
    uint16_t packet_count;	//	telemetry packet counter		2bytes	10
    uint16_t command_count;	//	command received counter		2bytes	12
  
    //states
    State::Flight mode;		//flight mode, flight/sim			1byte	14
    State::CTR_OP state;	//container state				1byte	15
    uint8_t mech_state;		//hex, bit0 dply mech, bit1 pq released		1byte	16
    uint8_t task_state;		//bitmaske	subtask state, 1bits each	1bytes (6bits)	17
  
    //sensors
    float altitude;		//[m]						4bytes	18
    float pressure;		//[pa]						4bytes	22
    float temperature;		//[C*]						4bytes	26
    float battery_voltage;	//[V]						4bytes	30
    float battery_current;	//[ma]						4bytes	34
    float attitude[3];		//[deg]						12bytes	38
    float rot_rate[3];		//[deg/s]	roll, pitch, yaw		12bytes	50
    float acc[3];		//[m/s^2]	east, south, down		12bytes	62
    uint8_t sensor_state;	//bitmask	sensor+cam status, 2bits each	1bytes (5bits)	74
  
    //cmd [uint16_t sequence_num] [uint8_t verb] [uint8_t noun] [uint32_t param]
    CMD::Ack cmd_rx;		//last received cmd				8bytes	75
    CMD::RxResult rx_result;	//how did receiving/parsing of the cmd go?	1byte	83
    CMD::Ack cmd_exec;		//last executed cmd				8bytes	84
    
    uint8_t reserved[34];	//reserved bytes				34bytes	92
    
    uint16_t crc;		//crc16-citt checksum				2bytes	126
    
  };//struct CTR_Telemetry								128
  
  
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
  

  //crc helper (packet verification)
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
               const Sensor::IMU& imu, uint16_t sensor_state,
               State::Flight mode, State::CTR_OP state, uint8_t mech_state, uint8_t task_state,
               CMD::Ack cmd_rx, CMD::RxResult rx_result, CMD::Ack cmd_exec);
               
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
