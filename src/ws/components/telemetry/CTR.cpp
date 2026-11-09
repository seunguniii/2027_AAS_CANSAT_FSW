#include "Telemetry.hpp"

#include <cstdint>
#include <algorithm>

#include "esp_timer.h"

#include "Msg.hpp"
#include "Sensors.hpp"
#include "Nodes.hpp"
#include "States.hpp"
#include "CMD.hpp"
#include "CRC.hpp"

namespace Telemetry{
  uint16_t ctr_packet_count = 1;
  
  CTR buildCTRTelemetry(const Msg::Node& this_node,
                        const Sensor::Barometer& barometer,
                        const Sensor::PowerMonitor& power_monitor,
                        const Sensor::IMU& imu,
                        uint16_t sensor_state, 
                        FlightMode mode, CTR_OpState state, uint8_t mech_state, uint16_t task_state,
                        CMD::Ack cmd_rx, CMD::Ack cmd_exec){
    //GET DATA
    //counters
    
    //CONSTRUCT TELEMETRY
    CTR telemetry{};
    
    //time since boot, convert [us] to [ms]
    //TODO HHMMSSmmm format
    telemetry.mission_time = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    
    //counters
    telemetry.packet_count = ctr_packet_count;
    telemetry.command_count = ctr_packet_count; //dummy
    
    //states
    telemetry.mode = mode;
    telemetry.state = state;
    telemetry.mech_state = mech_state;
    telemetry.task_state = task_state;
    
    //sensors
    telemetry.altitude = barometer.altitude;
    telemetry.pressure = barometer.pressure;
    telemetry.temperature = barometer.temperature;
    telemetry.battery_voltage = power_monitor.voltage;
    telemetry.battery_current = power_monitor.current;
    std::copy(imu.accel, imu.accel + 3, telemetry.acc);
    std::copy(imu.rot_rate, imu.rot_rate + 3, telemetry.rot_rate);
    telemetry.sensor_state = sensor_state;
    
    //cmd
    telemetry.cmd_rx = cmd_logger.echo;
    telemetry.cmd_exec = cmd_echo;
    
    uitls::buildCRC(telemetry);
    
    return telemetry;
  }


  bool sendCTR(const Msg::Node& this_node, const Msg::Node& peer_node,
               const Sensor::Barometer& barometer, 
               const Sensor::PowerMonitor& power_monitor,
               const Sensor::IMU& imu,
               uint16_t sensor_state,
               FlightMode mode, CTR_OpState state, uint8_t mech_state, uint16_t task_state,
               CMD::Ack cmd_rx, CMD::Ack cmd_exec){
    CTR telemetry = buildCTRTelemetry(this_node, barometer, power_monitor, imu, sensor_state,
                                      mode, state, mech_state, task_state, cmd_rx, cmd_exec);
    ctr_packet_count++;
    return this_node.send(telemetry, peer_node.getAddress());  
  }
}
