#pragma once
#include <cstdint>

enum class FlightMode: uint8_t{
  F = 0,	//flight
  S = 1,	//simulation
};


//CTR operation states denote stages, not success checkers
enum class CTR_OpState: uint8_t{//Container Operation State
  LAUNCH_PAD = 0,		//waiting for cmd/launch
  ASCENT = 1,			//(detected) ascent of rocket
  APOGEE = 2,			//(detected) peak alt
  PQ_RELEASE = 3,		//PQ release success
};


//Sensor health check
enum class SensorHealth: uint8_t {
  HEALTHY = 0,
  SUSPECT = 1,
  FAULT = 2,
  NOT_READY = 3,
};
