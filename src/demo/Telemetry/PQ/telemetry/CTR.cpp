#include "Telemetry.h"

#include<cstdint>

#include "esp_timer.h"

#include "Msg.h"
#include "Sensors.h"
#include "Nodes.h"
#include "States.h"

namespace Telemetry{
  uint16_t packet_count = 1;
  
  CTR buildCTRTelemetry(const Msg::Node& this_node,
                        const Sensor::Barometer& barometer,
                        const Sensor::PowerMonitor& power_monitor,
                        FlightMode mode, CTR_OpState state, uint8_t mech_state){    
    //GET DATA
    //counters
    //TODO CMD::CTR cmd_logger; //command logger
    //TODO this_node.get(cmd_logger); //gives cmd count & last cmd(echo)
    //packet_count handled internally
    
    //CONSTRUCT TELEMETRY
    CTR telemetry{};
    
    //time since boot, convert [us] to [ms]
    telemetry.mission_time = static_cast<uint32_t>(esp_timer_get_time() / 1000LL);
    
    //counters
    telemetry.packet_count = packet_count;
    //telemetry.command_count = cmd_logger.count;
    telemetry.command_count = packet_count; //dummy
    
    //states
    telemetry.mode = mode;
    telemetry.state = state;
    telemetry.mech_state = mech_state;
    
    //sensors
    telemetry.altitude = barometer.altitude;
    telemetry.pressure = barometer.pressure;
    telemetry.temperature = barometer.temperature;
    telemetry.battery_voltage = power_monitor.voltage;
    telemetry.battery_current = power_monitor.current;
    
    //miscellaneous
    //telemetry.cmd_echo = cmd_logger.echo; TODO
    telemetry.cmd_echo = CMD::CTR::INIT; //dummy
    
    buildCRC(telemetry);
    
    return telemetry;
  }


  bool sendCTR(const Msg::Node& this_node, const Msg::Node& peer_node,
               const Sensor::Barometer& barometer, 
               const Sensor::PowerMonitor& power_monitor,
               FlightMode mode, CTR_OpState state, uint8_t mech_state){
    CTR telemetry = buildCTRTelemetry(this_node, barometer, power_monitor,
                                      mode, state, mech_state);
    packet_count++;
    return this_node.send(telemetry, peer_node.getAddress());  
  }
}
