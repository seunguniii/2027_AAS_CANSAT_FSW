#include "Telemetry.h"

#include<cstdint>

#include "esp_timer.h"

#include "Msg.h"
#include "Sensors.h"
#include "Nodes.h"
#include "States.h"

namespace Telemetry{
  uint16_t pq_packet_count = 1;
  
  PQ buildPQTelemetry(const Msg::Node& this_node,
                      const Sensor::Barometer& barometer,
                      const Sensor::PowerMonitor& power_monitor,
                      const Sensor::IMU& imu,
                      const Sensor::Magnetometer& magnetometer,
                      const Sensor::GNSS& gnss,
                      const Sensor::PowerMonitor& sp1_monitor,
                      const Sensor::PowerMonitor& sp2_monitor,
                      FlightMode mode, uint8_t mech_state){    
                      
    //GET DATA
    //counters
    //TODO CMD::PQ cmd_logger; //command logger
    //TODO this_node.get(cmd_logger); //gives cmd count & last cmd(echo)
    //packet_count handled internally
    
    //CONSTRUCT TELEMETRY
    PQ telemetry{};
    
    //time since boot, convert [us] to [ms]
    telemetry.mission_time = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    
    //counters
    telemetry.packet_count = pq_packet_count;
    //telemetry.command_count = cmd_logger.count;
    telemetry.command_count = pq_packet_count; //dummy
    
    //states
    telemetry.mode = mode;
    telemetry.mech_state = mech_state;
    
    //sensors
    telemetry.altitude = barometer.altitude;
    telemetry.pressure = barometer.pressure;
    telemetry.temperature = barometer.temperature;
    telemetry.voltage = power_monitor.voltage;
    telemetry.current = power_monitor.current;
    std::memcpy(telemetry.rot_rate, imu.rot_rate, sizeof(telemetry.rot_rate));
    std::memcpy(telemetry.acc, imu.accel, sizeof(telemetry.acc));
    std::memcpy(telemetry.mag, magnetometer.mag, sizeof(telemetry.mag));
    telemetry.gnss_time = gnss.gnss_time;
    std::memcpy(telemetry.gnss_pos, gnss.position, sizeof(telemetry.gnss_pos));
    telemetry.gnss_sats = gnss.sats;
    telemetry.solar_1 = sp1_monitor.voltage;
    telemetry.solar_2 = sp2_monitor.voltage;    
    
    //miscellaneous
    //telemetry.cmd_echo = cmd_logger.echo; TODO
    telemetry.cmd_echo = CMD::PQ::INIT; //dummy
    telemetry.image_stabilization = CMD::PQ::IMG_STB; //dummy
    telemetry.science_exp = CMD::PQ::SCI_EXP; //dummy
    
    buildCRC(telemetry);
    
    return telemetry;
  }


  bool sendPQ(const Msg::Node& this_node, const Msg::Node& peer_node,
              const Sensor::Barometer& barometer, 
              const Sensor::PowerMonitor& power_monitor,
              const Sensor::IMU& imu,
              const Sensor::Magnetometer& magnetometer,
              const Sensor::GNSS& gnss,
              const Sensor::PowerMonitor& sp1_monitor,
              const Sensor::PowerMonitor& sp2_monitor,
              FlightMode mode, uint8_t mech_state){

    PQ telemetry = buildPQTelemetry(this_node,
                                    barometer,
                                    power_monitor,
                                    imu,
                                    magnetometer,
                                    gnss,
                                    sp1_monitor,
                                    sp2_monitor,
                                    mode, mech_state);
    pq_packet_count++;
    return this_node.send(telemetry, peer_node.getAddress());  
  }
}
