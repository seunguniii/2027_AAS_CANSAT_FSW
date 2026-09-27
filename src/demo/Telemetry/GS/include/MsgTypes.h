#pragma once

#include <cstdint>

#include "Sensors.h"
#include "CMD.h"
#include "Test.h"
#include "Telemetry.h"

namespace Msg{
  enum class MsgType: uint16_t{
    //sensors (0-49)
    IMU = 0,
    BAROMETER = 1,
    POWER_MONITOR = 2,
    MAGNETOMETER = 3,
    GNSS = 4,
    
    //telemetry (60 - 89)
    CTR_TELEMETRY = 60,
    PQ_TELEMETRY = 61,
    
    //commands (100 - 149)
    CTR_CMD = 100,
    PQ_CMD = 101,
    
    //test (200 - 255)
    TEST = 200,
  };
  
  
  //map msg type
  //no default value for compile error check of msg validity
  template <typename T>
  struct IDof;
  
  //SENSOR
  template <>
  struct IDof<Sensor::IMU>{
    static constexpr MsgType type = MsgType::IMU;
  };
  template <>
  struct IDof<Sensor::Barometer>{
    static constexpr MsgType type = MsgType::BAROMETER;
  };
  template <>
  struct IDof<Sensor::PowerMonitor>{
    static constexpr MsgType type = MsgType::POWER_MONITOR;
  };
  template <>
  struct IDof<Sensor::Magnetometer>{
    static constexpr MsgType type = MsgType::MAGNETOMETER;
  };
  template <>
  struct IDof<Sensor::GNSS>{
    static constexpr MsgType type = MsgType::GNSS;
  };
  
  
  //TELEMETRY
  template <>
  struct IDof<Telemetry::CTR>{
    static constexpr MsgType type = MsgType::CTR_TELEMETRY;
  };
  template <>
  struct IDof<Telemetry::PQ>{
    static constexpr MsgType type = MsgType::PQ_TELEMETRY;
  };
  
  
  //CMD
  template <>
  struct IDof<CMD::CTR>{
    static constexpr MsgType type = MsgType::CTR_CMD;
  };
  template <>
  struct IDof<CMD::PQ>{
    static constexpr MsgType type = MsgType::PQ_CMD;
  };
  
  //Test
  template <>
  struct IDof<Test::MsgTest>{
    static constexpr MsgType type = MsgType::TEST;
  };
}//namespace Msg
