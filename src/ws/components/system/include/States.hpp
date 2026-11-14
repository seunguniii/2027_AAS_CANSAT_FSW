#pragma once
#include <cstdint>

namespace State{
  enum class Health: uint8_t{
    HEALTHY = 0,
    FAULT = 1,
  };
  
  enum class Flight: uint8_t{
    FLIGHT = 0,
    SIMULATION = 1,
  };
  
  enum CTR_OP: uint8_t{//Container Operation State
    LAUNCH_PAD = 0,		//waiting for cmd/launch
    ASCENT = 1,			//(detected) ascent of rocket
    APOGEE = 2,			//(detected) peak alt
    PQ_RELEASE = 3,		//PQ release (success)?
  };
  
  enum Mech: uint8_t{
    STANDBY = 0,
    DEPLOYED = 1,
  };
}


//TODO delete all below after code is cleaned up---------------
enum class FlightMode: uint8_t{
  FLIGHT = 0,
  SIMULATION = 1,
};


//CTR operation states denote stages, not success checkers
enum class CTR_OpState: uint8_t{//Container Operation State
  LAUNCH_PAD = 0,		//waiting for cmd/launch
  ASCENT = 1,			//(detected) ascent of rocket
  APOGEE = 2,			//(detected) peak alt
  PQ_RELEASE = 3,		//PQ release success
};


//Sensor health check
namespace Sensor{
  enum class Health: uint8_t {
    HEALTHY = 0,
    SUSPECT = 1,
    FAULT = 2,
    NOTREADY = 3,
  };
}

