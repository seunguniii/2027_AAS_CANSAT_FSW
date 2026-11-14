#include "Telemetry.hpp"

#include <cstdint>
#include <algorithm>
#include <cmath>

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
                        CMD::Ack cmd_rx, CMD::RxResult rx_result, CMD::Ack cmd_exec){
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
    
    float attitude[3]; //roll pitch yaw
    float qx = imu.q[0]; float qy = imu.q[1]; 
    float qz = imu.q[2]; float qw = imu.q[3];
    attitude[0] = atan2(2*(qw*qx + qy*qx), 1 - 2*(qx*qx + qy*qy)) * 180/M_PI;
    attitude[1] = asin(2*(qw*qy - qz*qx)) * 180/M_PI;
    attitude[2] = atan2(2*(qw*qz + qx*qy), 1 - 2*(qy*qy + qz*qz)) * 180/M_PI;
    std::memcpy(telemetry.attitude, attitude, sizeof(telemetry.attitude));    
    std::memcpy(telemetry.rot_rate, imu.rot_rate, sizeof(telemetry.rot_rate));
    std::memcpy(telemetry.acc, imu.accel, sizeof(telemetry.acc));
    telemetry.sensor_state = sensor_state;
    
    //cmd
    telemetry.cmd_rx = cmd_rx;
    telemetry.rx_result = rx_result;
    telemetry.cmd_exec = cmd_exec;
    
    utils::buildCRC(telemetry);
    
    return telemetry;
  }


  bool sendCTR(const Msg::Node& this_node, const Msg::Node& peer_node,
               const Sensor::Barometer& barometer, 
               const Sensor::PowerMonitor& power_monitor,
               const Sensor::IMU& imu,
               uint16_t sensor_state,
               FlightMode mode, CTR_OpState state, uint8_t mech_state, uint16_t task_state,
               CMD::Ack cmd_rx, CMD::RxResult rx_result, CMD::Ack cmd_exec){
    CTR telemetry = buildCTRTelemetry(this_node, barometer, power_monitor, imu, sensor_state,
                                      mode, state, mech_state, task_state, cmd_rx, rx_result, cmd_exec);
    ctr_packet_count++;
    return this_node.send(telemetry, peer_node.getAddress());  
  }
}
